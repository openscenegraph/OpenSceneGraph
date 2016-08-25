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

#include <osgAnimation/Timeline>
#include <osgAnimation/TimelineAnimationManager>

using namespace osgAnimation;

TimelineAnimationManager::TimelineAnimationManager()
{
    _timeline = new Timeline;
}

TimelineAnimationManager::TimelineAnimationManager(const AnimationManagerBase& manager) : AnimationManagerBase(manager)
{
    _timeline = new Timeline;
}

TimelineAnimationManager::TimelineAnimationManager(const TimelineAnimationManager& nc,const osg::CopyOp& co):
    osg::Object(nc,co),
    osg::Callback(nc,co),
    AnimationManagerBase(nc, co)
{
    _timeline = new Timeline(*nc.getTimeline(), co);
}

void TimelineAnimationManager::update(double time)
{
    // clearTargets();
    _timeline->setAnimationManager(this);
    _timeline->update(time);
}
