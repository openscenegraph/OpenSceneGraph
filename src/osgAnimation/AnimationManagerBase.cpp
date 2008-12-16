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
#include <osgAnimation/Assert>

using namespace osgAnimation;


AnimationManagerBase::~AnimationManagerBase() {}

AnimationManagerBase::AnimationManagerBase()
{
    setUpdateCallback(new UpdateCallback); 
    _needToLink = false; 
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



void AnimationManagerBase::link()
{
    LinkVisitor linker(_animations);
    accept(linker);
    _needToLink = false;
    buildTargetReference();
}


osgAnimation::AnimationMap AnimationManagerBase::getAnimationMap() const
{
    osgAnimation::AnimationMap map;
    for (AnimationList::const_iterator it = _animations.begin(); it != _animations.end(); it++)
        map[(*it)->getName()] = *it;
    return map;
}

