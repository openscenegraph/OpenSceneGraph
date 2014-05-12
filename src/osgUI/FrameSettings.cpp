/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2014 Robert Osfield
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

#include <osgUI/FrameSettings>

using namespace osgUI;

FrameSettings::FrameSettings():
    _shape(FrameSettings::NO_FRAME),
    _shadow(FrameSettings::PLAIN),
    _lineWidth(0.01)
{
}

FrameSettings::FrameSettings(const FrameSettings& frameSettings, const osg::CopyOp& copyop):
    osg::Object(frameSettings, copyop),
    _shape(frameSettings._shape),
    _shadow(frameSettings._shadow),
    _lineWidth(frameSettings._lineWidth)
{
}
