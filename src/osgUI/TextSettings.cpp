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

#include <osgUI/TextSettings>
#include <osg/Geode>
#include <osgText/Text>

using namespace osgUI;

TextSettings::TextSettings():
    _characterSize(1.0)
{
}

TextSettings::TextSettings(const TextSettings& textSettings, const osg::CopyOp& copyop):
    osg::Object(textSettings, copyop),
    _font(textSettings._font),
    _characterSize(textSettings._characterSize)
{
}

