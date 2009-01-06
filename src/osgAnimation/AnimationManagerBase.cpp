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

#include <osgAnimation/AnimationManagerBase>
#include <osgAnimation/LinkVisitor>

using namespace osgAnimation;

AnimationManagerBase::~AnimationManagerBase() {}

AnimationManagerBase::AnimationManagerBase()
{
    _needToLink = false; 
}

void AnimationManagerBase::clearTargets()
{
    for (TargetSet::iterator it = _targets.begin(); it != _targets.end(); it++)
        (*it).get()->reset();
}
void AnimationManagerBase::normalizeTargets()
{
    for (TargetSet::iterator it = _targets.begin(); it != _targets.end(); it++)
        (*it).get()->normalize();
}

void AnimationManagerBase::operator()(osg::Node* node, osg::NodeVisitor* nv)
{ 
    if (nv && nv->getVisitorType() == osg::NodeVisitor::UPDATE_VISITOR) 
    {
        if (needToLink())
        {
            /** manager need to link, it means that an animation has been added
                so we need to relink all item animated with all animations.
                We apply the linker visitor on the manager node to affect
                all its children.
                But it should not be done here, it should be done in the
                update of AnimationManager
            */
            link(node);
        }
        const osg::FrameStamp* fs = nv->getFrameStamp();
        update(fs->getSimulationTime());
    }
    traverse(node,nv);
}


AnimationManagerBase::AnimationManagerBase(const AnimationManagerBase& b, const osg::CopyOp& copyop) : osg::NodeCallback(b,copyop) 
{
    _animations = b._animations;
    _targets = b._targets;
    _needToLink = b._needToLink;
}

void AnimationManagerBase::buildTargetReference()
{
    _targets.clear();
    for( AnimationList::iterator iterAnim = _animations.begin(); iterAnim != _animations.end(); ++iterAnim ) 
    {
        Animation* anim = (*iterAnim).get();
        for (ChannelList::iterator it = anim->getChannels().begin();
             it != anim->getChannels().end();
             it++)
            _targets.insert((*it)->getTarget());
    }
}


void AnimationManagerBase::registerAnimation (Animation* animation)
{
    _needToLink = true;
    _animations.push_back(animation);
    buildTargetReference();
}

bool AnimationManagerBase::needToLink() const { return _needToLink; }



void AnimationManagerBase::link(osg::Node* subgraph)
{
    LinkVisitor linker(_animations);
    subgraph->accept(linker);
    _needToLink = false;
    buildTargetReference();
}
