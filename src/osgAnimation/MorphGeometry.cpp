/*  -*-c++-*-
 *  Copyright (C) 2008 Cedric Pinson <cedric.pinson@plopbyte.net>
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

#include <osg/Geode>
#include <osgAnimation/MorphGeometry>
#include <osgAnimation/RigGeometry>

#include <sstream>

using namespace osgAnimation;

MorphGeometry::MorphGeometry() :
    _dirty(false),
    _method(NORMALIZED),
    _positionSource(0),_normalSource(0),
    _morphNormals(true)
{
    setUseDisplayList(false);
    setUpdateCallback(new UpdateMorphGeometry);
    setUseVertexBufferObjects(true);
    _morphTransformImplementation = new MorphTransformSoftware();
}

MorphGeometry::MorphGeometry(const osg::Geometry& g) :
    osg::Geometry(g, osg::CopyOp::DEEP_COPY_ARRAYS),
    _dirty(false),
    _method(NORMALIZED),
    _positionSource(0),_normalSource(0),
    _morphNormals(true)
{
    setUseDisplayList(false);
    setUpdateCallback(new UpdateMorphGeometry);
    setUseVertexBufferObjects(true);
    _morphTransformImplementation = new MorphTransformSoftware();
}

MorphGeometry::MorphGeometry(const MorphGeometry& b, const osg::CopyOp& copyop) :
    osg::Geometry(b,copyop),
    _morphTransformImplementation(osg::clone(b._morphTransformImplementation.get(), copyop)),
    _dirty(b._dirty),
    _method(b._method),
    _morphTargets(b._morphTargets),
    _positionSource(b._positionSource),
    _normalSource(b._normalSource),
    _morphNormals(b._morphNormals)
{
    setUseDisplayList(false);
    setUseVertexBufferObjects(true);
}

UpdateMorph::UpdateMorph(const UpdateMorph& apc,const osg::CopyOp& copyop) :
    osg::Object(apc, copyop),
    osg::Callback(apc, copyop),
    AnimationUpdateCallback<osg::NodeCallback>(apc, copyop)
{
    _targetNames=apc._targetNames;
}

UpdateMorph::UpdateMorph(const std::string& name) : AnimationUpdateCallback<osg::NodeCallback>(name)
{
}

/** Callback method called by the NodeVisitor when visiting a node.*/
void UpdateMorph::operator()(osg::Node* node, osg::NodeVisitor* nv)
{
    if (nv && nv->getVisitorType() == osg::NodeVisitor::UPDATE_VISITOR)
    {
        osg::Geode* geode = dynamic_cast<osg::Geode*>(node);
        if (geode)
        {
            unsigned int numDrawables = geode->getNumDrawables();
            for (unsigned int i = 0; i != numDrawables; ++i)
            {
                osg::Drawable *drw = geode->getDrawable(i);
                osgAnimation::RigGeometry *rig = dynamic_cast<osgAnimation::RigGeometry*>(drw);
                if(rig && rig->getSourceGeometry())
                    drw = rig->getSourceGeometry();

                osgAnimation::MorphGeometry* morph = dynamic_cast<osgAnimation::MorphGeometry*>(drw);
                if (morph)
                {
                    // Update morph weights
                    std::map<int, osg::ref_ptr<osgAnimation::FloatTarget> >::iterator iter = _weightTargets.begin();
                    while (iter != _weightTargets.end())
                    {
                        if (iter->second->getValue() >= 0)
                        {
                            morph->setWeight(iter->first, iter->second->getValue());
                        }
                        ++iter;
                    }
                }
            }
        }
    }
    traverse(node,nv);
}

bool UpdateMorph::needLink() const
{
    // the idea is to return true if nothing is linked
    return (_weightTargets.size() == 0);
}

bool UpdateMorph::link(osgAnimation::Channel* channel)
{
    // Typically morph geometries only have the weights for morph targets animated

    std::istringstream iss(channel->getName());

    int weightIndex;
    iss >> weightIndex;

    if (iss.fail())
    {
        return false;
    }

    if (weightIndex >= 0)
    {
        osgAnimation::FloatTarget* ft = _weightTargets[weightIndex].get();
        if (!ft)
        {
            ft = new osgAnimation::FloatTarget;
            _weightTargets[weightIndex] = ft;
        }
        return channel->setTarget(ft);
    }
    else
    {
        OSG_WARN << "Channel " << channel->getName() << " does not contain a valid symbolic name for this class" << std::endl;
    }
    return false;
}

int UpdateMorph::link(Animation* animation)
{
    if (getNumTarget() == 0)
    {
        osg::notify(osg::WARN) << "An update callback has no name, it means it could link only with \"\" named Target, often an error, discard" << std::endl;
        return 0;
    }

    unsigned int nbLinks = 0;
    for (ChannelList::iterator channel = animation->getChannels().begin();
         channel != animation->getChannels().end();
         ++channel)
    {
        std::string targetName = (*channel)->getTargetName();
        for(int i = 0, num = getNumTarget(); i < num; ++i) {
            if (targetName == getTargetName(i))
            {
                AnimationUpdateCallbackBase* a = this;
                a->link((*channel).get());
                nbLinks++;
            }
        }
    }
    return nbLinks;
}
