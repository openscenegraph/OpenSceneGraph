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
#include <osg/ClampColor>
#include <osg/GLExtensions>
#include <osg/State>
#include <osg/Notify>
#include <osg/buffered_value>


using namespace osg;

ClampColor::ClampColor():
   _clampVertexColor(GL_FIXED_ONLY),
   _clampFragmentColor(GL_FIXED_ONLY),
   _clampReadColor(GL_FIXED_ONLY)
{
}

ClampColor::ClampColor(GLenum vertexMode, GLenum fragmentMode, GLenum readMode):
   _clampVertexColor(vertexMode),
   _clampFragmentColor(fragmentMode),
   _clampReadColor(readMode)
{
}

ClampColor::~ClampColor()
{
}

void ClampColor::apply(State& state) const
{

   // get the contextID (user defined ID of 0 upwards) for the
    // current OpenGL context.
    const GLExtensions* extensions = state.get<GLExtensions>();
    if (!extensions->isClampColorSupported)
    {
        OSG_WARN<<"Warning: ClampColor::apply(..) failed, ClampColor is not support by OpenGL driver."<<std::endl;
        return;
    }

    extensions->glClampColor(GL_CLAMP_VERTEX_COLOR, _clampVertexColor);
    extensions->glClampColor(GL_CLAMP_FRAGMENT_COLOR, _clampFragmentColor);
    extensions->glClampColor(GL_CLAMP_READ_COLOR, _clampReadColor);
}
