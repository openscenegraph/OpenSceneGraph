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
#include <osg/ColorMaski>
#include <osg/GLExtensions>
#include <osg/State>

using namespace osg;

ColorMaski::ColorMaski():
    _index(0)
{
}

ColorMaski::~ColorMaski()
{
}

void ColorMaski::setIndex(unsigned int buf)
{
    if (_index==buf) return;

    ReassignToParents needToReassingToParentsWhenMemberValueChanges(this);

    _index = buf;
}

void ColorMaski::apply(State& state) const
{
    const GLExtensions* extensions = state.get<GLExtensions>();
    if (extensions->glColorMaski)
    {
        extensions->glColorMaski((GLuint)_index, (GLboolean)_red,(GLboolean)_green,(GLboolean)_blue,(GLboolean)_alpha);
    }
    else
    {
        OSG_WARN<<"Warning: ColorMaski::apply(..) failed, glColorMaski is not support by OpenGL driver."<<std::endl;
    }
}

