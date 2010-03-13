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

#include <osgAnimation/BasicAnimationManager>
#include <osgAnimation/LinkVisitor>

using namespace osgAnimation;

BasicAnimationManager::BasicAnimationManager()
: _lastUpdate(0.0) 
{
}

BasicAnimationManager::BasicAnimationManager(const AnimationManagerBase& b, const osg::CopyOp& copyop) 
: AnimationManagerBase(b,copyop)
, _lastUpdate(0.0) 
{
}

BasicAnimationManager::~BasicAnimationManager() 
{
}

void BasicAnimationManager::stopAll()
{
    // loop over all playing animation
    for( AnimationLayers::iterator iterAnim = _animationsPlaying.begin(); iterAnim != _animationsPlaying.end(); ++iterAnim ) 
    {
        AnimationList& list = iterAnim->second;
        for (AnimationList::iterator it = list.begin(); it != list.end(); ++it)
            (*it)->resetTargets();
    }
    _animationsPlaying.clear();
}

void BasicAnimationManager::playAnimation(Animation* pAnimation, int priority, float weight)
{
    if (!findAnimation(pAnimation))
        return;

    if ( isPlaying(pAnimation) )
        stopAnimation(pAnimation);
  
    _animationsPlaying[priority].push_back(pAnimation);
    // for debug
    //std::cout << "player Animation " << pAnimation->getName() << " at " << _lastUpdate << std::endl;
    pAnimation->setStartTime(_lastUpdate);
    pAnimation->setWeight(weight);
}

bool BasicAnimationManager::stopAnimation(Animation* pAnimation)
{
    // search though the layer and remove animation
    for( AnimationLayers::iterator iterAnim = _animationsPlaying.begin(); iterAnim != _animationsPlaying.end(); ++iterAnim )
    {
        AnimationList& list = iterAnim->second;
        for (AnimationList::iterator it = list.begin(); it != list.end(); ++it)
            if( (*it) == pAnimation )
            {
                (*it)->resetTargets();
                list.erase(it);
                return true;
            }
    }
    return false;
}


void BasicAnimationManager::update (double time)
{
    _lastUpdate = time; // keep time of last update

    // could filtered with an active flag
    for (TargetSet::iterator it = _targets.begin(); it != _targets.end(); ++it)
        (*it).get()->reset();

    // update from high priority to low priority
    for( AnimationLayers::reverse_iterator iterAnim = _animationsPlaying.rbegin(); iterAnim != _animationsPlaying.rend(); ++iterAnim )
    {
        // update all animation
        std::vector<int> toremove;
        int priority = iterAnim->first;
        AnimationList& list = iterAnim->second;
        for (unsigned int i = 0; i < list.size(); i++)
        {
            if (! list[i]->update(time, priority)) 
            {
                // debug
                // std::cout << list[i]->getName() << " finished at " << time << std::endl;
                toremove.push_back(i);
            } else 
            {
                // debug
                //std::cout << list[i]->getName() << " updated" << std::endl;
            }
        }

        // remove finished animation
        while (!toremove.empty())
        {
            list.erase(list.begin() + toremove.back());
            toremove.pop_back();
        }
    }
}


bool BasicAnimationManager::findAnimation(Animation* pAnimation)
{
    for( AnimationList::const_iterator iterAnim = _animations.begin(); iterAnim != _animations.end(); ++iterAnim ) 
    {
        if ( (*iterAnim) == pAnimation )
            return true;
    }
    return false;
}


bool BasicAnimationManager::isPlaying(Animation* pAnimation)
{
    for( AnimationLayers::iterator iterAnim = _animationsPlaying.begin(); iterAnim != _animationsPlaying.end(); ++iterAnim )
    {
        AnimationList& list = iterAnim->second;
        for (AnimationList::iterator it = list.begin(); it != list.end(); ++it)
            if ( (*it) == pAnimation )
                return true;
    }
    return false;
}

bool BasicAnimationManager::isPlaying(const std::string& name)
{
    // loop over all playing animation
    for( AnimationLayers::iterator iterAnim = _animationsPlaying.begin(); iterAnim != _animationsPlaying.end(); ++iterAnim )
    {
        AnimationList& list = iterAnim->second;
        for (AnimationList::iterator it = list.begin(); it != list.end(); ++it)
            if ( (*it)->getName() == name )
                return true;
    }
    return false;
}
