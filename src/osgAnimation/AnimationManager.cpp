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
#include <osgAnimation/NodeVisitor>
#include <osgAnimation/Assert>
#include <sys/time.h>

using namespace osgAnimation;

static unsigned int getGlobaleTime()
{
  struct timeval now;
  gettimeofday(&now, 0);
  return now.tv_sec * 1000 + now.tv_usec / 1000;
}

static float getLocalTime()
{
  static unsigned int reference = getGlobaleTime();
  return (getGlobaleTime()-reference)/1000.0;
}

AnimationManager::~AnimationManager() {}


void AnimationManager::stopAll()
{
  while (!_listAnimPlaying.empty())
    stopAnimation((*_listAnimPlaying.begin()).get());
}

void AnimationManager::initTimer()
{
  _lastUpdate = getLocalTime();
}

AnimationManager::AnimationManager() 
{
  _lastUpdate = -1; 
  initTimer(); 
  setUpdateCallback(new UpdateCallback); 
  _needToLink = false; 
}

osgAnimation::AnimationMap AnimationManager::getAnimationMap() const
{
  osgAnimation::AnimationMap map;
  for (AnimationList::const_iterator it = _listAnimation.begin(); it != _listAnimation.end(); it++)
    map[(*it)->getName()] = *it;
  return map;
}

void AnimationManager::update()
{
  update(getLocalTime());
}

void AnimationManager::playAnimation(Animation* pAnimation, float weight)
{
  bool r = findAnimation(pAnimation);
  OSGANIMATION_ASSERT(r && "This animation is not registered");

  if ( isPlaying(pAnimation) )
    stopAnimation(pAnimation);
  
  _listAnimPlaying.push_back(pAnimation); 
  pAnimation->setStartTime(_lastUpdate);
  pAnimation->setWeight(weight);  
}

bool AnimationManager::stopAnimation(Animation* pAnimation)
{
  // search though the list and remove animation
  for( AnimationList::iterator iterAnim = _listAnimPlaying.begin(); iterAnim != _listAnimPlaying.end(); ++iterAnim ) {
    if( (*iterAnim) == pAnimation ) {
      (*iterAnim)->resetTargets();
      _listAnimPlaying.erase(iterAnim);
      return true;
    }
  }
  return false;
}


void AnimationManager::buildTargetReference()
{
  _targets.clear();
  for( AnimationList::iterator iterAnim = _listAnimation.begin(); iterAnim != _listAnimation.end(); ++iterAnim ) {
    for (ChannelList::iterator it = (*iterAnim)->getChannels().begin();
         it != (*iterAnim)->getChannels().end();
         it++)
      _targets.insert((*it)->getTarget());
  }
}

void AnimationManager::update (float time)
{
  _lastUpdate = time;

  // could filtered with an active flag
  for (TargetSet::iterator it = _targets.begin(); it != _targets.end(); it++)
      (*it).get()->reset();

  // update all target
  std::vector<int> toremove;
  for( int i = 0; i < (int)_listAnimPlaying.size(); i++ )
      if (! _listAnimPlaying[i]->update(time))
          toremove.push_back(i);

  // erase finished animation
  while (!toremove.empty())
  {
      _listAnimPlaying.erase(_listAnimPlaying.begin() + toremove.back());
      toremove.pop_back();
  }

  for (TargetSet::iterator it = _targets.begin(); it != _targets.end(); it++)
    (*it).get()->normalize();
}

void AnimationManager::registerAnimation (Animation* pAnimation)
{
  _needToLink = true;
  _listAnimation.push_back(pAnimation);
  buildTargetReference();
}

bool AnimationManager::needToLink() const { return _needToLink; }



void AnimationManager::link()
{
  LinkVisitor linker(_listAnimation);
  accept(linker);
  _needToLink = false;
  buildTargetReference();
}

bool AnimationManager::findAnimation(Animation* pAnimation) 
{
    for( AnimationList::const_iterator iterAnim = _listAnimation.begin(); iterAnim != _listAnimation.end(); ++iterAnim ) 
    {
        if ( (*iterAnim) == pAnimation )
            return true;
    }
    return false;
}

bool AnimationManager::isPlaying(Animation* pAnimation) 
{
    for( AnimationList::const_iterator iterAnim = _listAnimPlaying.begin(); iterAnim != _listAnimPlaying.end(); ++iterAnim ) 
    {
        if ( (*iterAnim) == pAnimation )
            return true;
    }
    return false;
}

bool AnimationManager::isPlaying(const std::string& name)
{
    for( AnimationList::const_iterator iterAnim = _listAnimPlaying.begin(); iterAnim != _listAnimPlaying.end(); ++iterAnim ) 
    {
        if ( (*iterAnim)->getName() == name )
            return true;
    }
    return false;
}
