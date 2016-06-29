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

#include <osgAnimation/Channel>
using namespace osgAnimation;

Channel::Channel() {}
Channel::~Channel() {}
Channel::Channel(const Channel& channel) : osg::Object(channel),
                                           _targetName(channel._targetName),
                                           _name(channel._name)
{
}

const std::string& Channel::getName() const { return _name; }
void Channel::setName (const std::string& name) { _name = name; }

const std::string& Channel::getTargetName() const { return _targetName;}
void Channel::setTargetName (const std::string& name) { _targetName = name; }
