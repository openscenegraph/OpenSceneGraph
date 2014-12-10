/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield
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
#include <osg/BlendColor>
#include <osg/GLExtensions>
#include <osg/State>
#include <osg/Notify>
#include <osg/buffered_value>

using namespace osg;


BlendColor::BlendColor() :
    _constantColor(1.0f,1.0f,1.0f,1.0f)
{
}

BlendColor::BlendColor(const osg::Vec4& constantColor):
    _constantColor(constantColor)
{
}

BlendColor::~BlendColor()
{
}

void BlendColor::apply(State& state) const
{
    const GLExtensions* extensions = state.get<GLExtensions>();
    if (!extensions->isBlendColorSupported)
    {
        OSG_WARN<<"Warning: BlendColor::apply(..) failed, BlendColor is not support by OpenGL driver."<<std::endl;
        return;
    }

    extensions->glBlendColor(_constantColor[0], _constantColor[1], _constantColor[2], _constantColor[3]);
}
