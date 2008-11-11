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

#include <osgAnimation/Timeline>

using namespace osgAnimation;

// temporary
// the problem comes that the AnimationManagerBase should only a group
// and it's data should be in an update callback
struct TimelineAdaptator : public Timeline
{
    osg::ref_ptr<AnimationManagerTimeline> _manager;

    TimelineAdaptator(AnimationManagerTimeline* manager) : _manager(manager) {}
    void evaluate(unsigned int frame)
    {
        _manager->clearTargets();
        Timeline::evaluate(frame);
        _manager->normalizeTargets();
    }
};

AnimationManagerTimeline::AnimationManagerTimeline()
{
    _timeline = new TimelineAdaptator(this);
}

AnimationManagerTimeline::AnimationManagerTimeline(const AnimationManagerBase& manager) : AnimationManagerBase(manager)
{
    _timeline = new TimelineAdaptator(this);
}

void AnimationManagerTimeline::update(double time)
{
    _timeline->update(time);
}

