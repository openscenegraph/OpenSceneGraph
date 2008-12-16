/*  -*-c++-*- 
 *  Copyright (C) 2008 Cedric Pinson <mornifle@plopbyte.net>
 *
 * This library is open source and may be redistributed and/or modified under  
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or 
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * OpenSceneGraph Public License for more details.
*/

#include <osgAnimation/UpdateCallback>
#include <osg/MatrixTransform>
#include <osg/PositionAttitudeTransform>

using namespace osgAnimation;

osgAnimation::AnimationManagerBase* AnimationUpdateCallback::getAnimationManager() { return _manager.get(); }

AnimationUpdateCallback::AnimationUpdateCallback(const AnimationUpdateCallback& apc,const osg::CopyOp& copyop):
    osg::NodeCallback(apc, copyop),
    _manager(apc._manager) {}

int AnimationUpdateCallback::link(osgAnimation::Animation* animation)
{
    if (getName().empty())
        osg::notify(osg::WARN) << "An update callback has no name, it means it can link only with \"\" named Target, often an error" << std::endl;
    int nbLinks = 0;
    for (osgAnimation::ChannelList::iterator it = animation->getChannels().begin();
         it != animation->getChannels().end();
         it++)
    {
        std::string targetName = (*it)->getTargetName();
        if (targetName == getName()) 
        {
            link((*it).get());
            nbLinks++;
        }
    }
    return nbLinks;
}
    
void AnimationUpdateCallback::updateLink()
{
    if (_manager.valid())
    {
        if (needLink()) 
        {
            /** this item is not linked yet then we do it for all animation
                registered in the manager.
                Maybe this function should be on the manager side like
                _manager->linkItem(Bone);
            */
            const AnimationList& animationList = _manager->getAnimationList();
            for (AnimationList::const_iterator it = animationList.begin(); it != animationList.end(); it++)
                link(it->get());
            _manager->buildTargetReference();
        }
    }
}




UpdateTransform::UpdateTransform(const UpdateTransform& apc,const osg::CopyOp& copyop) 
    : AnimationUpdateCallback(apc, copyop),
      _euler(apc._euler),
      _position(apc._position),
      _scale(apc._scale)
{
}

UpdateTransform::UpdateTransform(const std::string& name) : AnimationUpdateCallback(name) 
{
    _euler = new osgAnimation::Vec3Target;
    _position = new osgAnimation::Vec3Target;
    _scale = new osgAnimation::Vec3Target(osg::Vec3(1,1,1));
}

/** Callback method called by the NodeVisitor when visiting a node.*/
void UpdateTransform::operator()(osg::Node* node, osg::NodeVisitor* nv)
{
    if (nv && nv->getVisitorType() == osg::NodeVisitor::UPDATE_VISITOR) 
    {
        osg::MatrixTransform* matrix = dynamic_cast<osg::MatrixTransform*>(node);
        if (matrix) 
        {
            update(*matrix);
        }
        else 
        {
            osg::PositionAttitudeTransform* pat = dynamic_cast<osg::PositionAttitudeTransform*>(node);
            if (pat)
                update(*pat);
        }
    }
    traverse(node,nv);
}

void UpdateTransform::update(osg::MatrixTransform& mat) 
{
    float z = _euler->getValue()[2];
    float x = _euler->getValue()[0];
    float y = _euler->getValue()[1];
    osg::Matrix m = 
        osg::Matrix::rotate(x,1.0,0.0,0.0) * 
        osg::Matrix::rotate(y,0.0,1.0,0.0) *
        osg::Matrix::rotate(z,0.0,0.0,1.0);
    mat.setMatrix(osg::Matrix::scale(_scale->getValue()) * 
                  m *
                  osg::Matrix::translate(_position->getValue()));
    mat.dirtyBound();
}

void UpdateTransform::update(osg::PositionAttitudeTransform& pat) 
{
    float heading = _euler->getValue()[0];
    float pitch = _euler->getValue()[1];
    float roll = _euler->getValue()[2];
    osg::Matrix m = osg::Matrix::rotate(roll,0.0,1.0,0.0) * osg::Matrix::rotate(pitch,1.0,0.0,0.0) * osg::Matrix::rotate(-heading,0.0,0.0,1.0);
    osg::Quat q = m.getRotate();

    pat.setPosition(_position->getValue());
    pat.setScale(_scale->getValue());
    pat.setAttitude(q);
    pat.dirtyBound();
}

bool UpdateTransform::needLink() const
{
    // the idea is to return true if nothing is linked
    return !((_position->getCount() + _euler->getCount() + _scale->getCount()) > 3);
}

bool UpdateTransform::link(osgAnimation::Channel* channel)
{
    if (channel->getName().find("euler") != std::string::npos) 
    {
        osgAnimation::Vec3LinearChannel* qc = dynamic_cast<osgAnimation::Vec3LinearChannel*>(channel);
        if (qc) 
        {
            qc->setTarget(_euler.get());
            return true;
        }
    }
    else if (channel->getName().find("position") != std::string::npos) 
    {
        osgAnimation::Vec3LinearChannel* vc = dynamic_cast<osgAnimation::Vec3LinearChannel*>(channel);
        if (vc) 
        {
            vc->setTarget(_position.get());
            return true;
        }
    }
    else if (channel->getName().find("scale") != std::string::npos) 
    {
        osgAnimation::Vec3LinearChannel* vc = dynamic_cast<osgAnimation::Vec3LinearChannel*>(channel);
        if (vc) 
        {
            vc->setTarget(_scale.get());
            return true;
        }
    } 
    else 
    {
        std::cerr << "Channel " << channel->getName() << " does not contain a valid symbolic name for this class" << std::endl;
    }
    return false;
}
