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
#include <limits.h>

using namespace osgAnimation;


Timeline::Timeline()
{
    _lastUpdate = 0;
    _currentFrame = 0;
    _fps = 25;
    _speed = 1.0;
    _state = Stop;
    _initFirstFrame = false;
    _previousFrameEvaluated = 0;
    _evaluating = 0;
    _numberFrame = UINT_MAX; // something like infinity
    setName("Timeline");
}

Timeline::Timeline(const Timeline& nc,const osg::CopyOp& op) : osg::Object(nc, op),
    _actions(nc._actions)
{
    _lastUpdate = 0;
    _currentFrame = 0;
    _fps = 25;
    _speed = 1.0;
    _state = Stop;
    _initFirstFrame = false;
    _previousFrameEvaluated = 0;
    _evaluating = 0;
    _numberFrame = UINT_MAX; // something like infinity
    setName("Timeline");
}
