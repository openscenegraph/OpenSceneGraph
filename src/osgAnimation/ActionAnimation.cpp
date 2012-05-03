/*  -*-c++-*-
 *  Copyright (C) 2009 Cedric Pinson <cedric.pinson@plopbyte.net>
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

#include <osgAnimation/ActionAnimation>

using namespace osgAnimation;

ActionAnimation::ActionAnimation() {}

ActionAnimation::ActionAnimation(const ActionAnimation& a, const osg::CopyOp& c) : Action(a,c) { _animation = a._animation;}

ActionAnimation::ActionAnimation(Animation* animation) : _animation(animation)
{
    Action::setDuration(animation->getDuration());
    setName(animation->getName());
}

void ActionAnimation::updateAnimation(unsigned int frame, int priority)
{
    _animation->update(frame * 1.0/_fps, priority);
}
