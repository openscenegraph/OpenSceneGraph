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

#include <osgAnimation/AnimationManager>
#include <osgAnimation/LinkVisitor>

using namespace osgAnimation;


AnimationManager::~AnimationManager() {}


void AnimationManager::stopAll()
{
    // loop over all playing animation
    for( AnimationLayers::iterator iterAnim = _animationsPlaying.begin(); iterAnim != _animationsPlaying.end(); ++iterAnim ) 
    {
        AnimationList& list = iterAnim->second;
        for (AnimationList::iterator it = list.begin(); it != list.end(); it++)
            (*it)->resetTargets();
    }
    _animationsPlaying.clear();
}

AnimationManager::AnimationManager()
{
    _lastUpdate = 0; 
}
void AnimationManager::playAnimation(Animation* pAnimation, int priority, float weight)
{
    if (!findAnimation(pAnimation))
    {
        return;
    }

    if ( isPlaying(pAnimation) )
        stopAnimation(pAnimation);
  
    _animationsPlaying[priority].push_back(pAnimation);
    pAnimation->setStartTime(_lastUpdate);
    pAnimation->setWeight(weight);
}

bool AnimationManager::stopAnimation(Animation* pAnimation)
{
    // search though the layer and remove animation
    for( AnimationLayers::iterator iterAnim = _animationsPlaying.begin(); iterAnim != _animationsPlaying.end(); ++iterAnim ) 
    {
        AnimationList& list = iterAnim->second;
        for (AnimationList::iterator it = list.begin(); it != list.end(); it++)
            if( (*it) == pAnimation )
            {
                (*it)->resetTargets();
                list.erase(it);
                return true;
            }
    }
    return false;
}


void AnimationManager::update (double time)
{
    if (!_lastUpdate)
        _lastUpdate = time;

    // could filtered with an active flag
    for (TargetSet::iterator it = _targets.begin(); it != _targets.end(); it++)
        (*it).get()->reset();


    // update from high priority to low priority
    for( AnimationLayers::reverse_iterator iterAnim = _animationsPlaying.rbegin(); iterAnim != _animationsPlaying.rend(); ++iterAnim )
    {
        // update all animation
        std::vector<int> toremove;
        AnimationList& list = iterAnim->second;
        for (unsigned int i = 0; i < list.size(); i++)
        {
            if (! list[i]->update(time))
                toremove.push_back(i);
        }

        // remove finished animation
        while (!toremove.empty())
        {
            list.erase(list.begin() + toremove.back());
            toremove.pop_back();
        }
    }

    for (TargetSet::iterator it = _targets.begin(); it != _targets.end(); it++)
        (*it).get()->normalize();
}


bool AnimationManager::findAnimation(Animation* pAnimation) 
{
    for( AnimationList::const_iterator iterAnim = _animations.begin(); iterAnim != _animations.end(); ++iterAnim ) 
    {
        if ( (*iterAnim) == pAnimation )
            return true;
    }
    return false;
}


bool AnimationManager::isPlaying(Animation* pAnimation) 
{
    for( AnimationLayers::iterator iterAnim = _animationsPlaying.begin(); iterAnim != _animationsPlaying.end(); ++iterAnim )
    {
        AnimationList& list = iterAnim->second;
        for (AnimationList::iterator it = list.begin(); it != list.end(); it++)
            if ( (*it) == pAnimation )
                return true;
    }
    return false;
}

bool AnimationManager::isPlaying(const std::string& name)
{
    // loop over all playing animation
    for( AnimationLayers::iterator iterAnim = _animationsPlaying.begin(); iterAnim != _animationsPlaying.end(); ++iterAnim )
    {
        AnimationList& list = iterAnim->second;
        for (AnimationList::iterator it = list.begin(); it != list.end(); it++)
            if ( (*it)->getName() == name )
                return true;
    }
    return false;
}
