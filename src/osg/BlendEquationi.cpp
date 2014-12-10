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
#include <osg/BlendEquationi>
#include <osg/GLExtensions>
#include <osg/State>

using namespace osg;

BlendEquationi::BlendEquationi():
    _index(0)
{
}


BlendEquationi::~BlendEquationi()
{
}

void BlendEquationi::apply(State& state) const
{
    const GLExtensions* extensions = state.get<GLExtensions>();
    if (_equationRGB == _equationAlpha)
    {
        if (extensions->glBlendEquationi)
        {
            extensions->glBlendEquationi(static_cast<GLuint>(_index), static_cast<GLenum>(_equationRGB));
        }
        else
        {
            OSG_WARN<<"Warning: BlendEquationi::apply(..) not supported by OpenGL driver." << std::endl;
        }
    }
    else
    {
        if (extensions->glBlendEquationSeparatei)
        {
            extensions->glBlendEquationSeparatei(static_cast<GLuint>(_index), static_cast<GLenum>(_equationRGB), static_cast<GLenum>(_equationAlpha));
        }
        else
        {
            OSG_WARN<<"Warning: BlendEquation::apply(..) failed, glBlendEquationSeparatei not supported by OpenGL driver." << std::endl;
        }
    }
}

