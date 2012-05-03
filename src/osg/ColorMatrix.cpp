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
#include <osg/GLExtensions>
#include <osg/ColorMatrix>
#include <osg/GL>
#include <osg/State>
#include <osg/Notify>

using namespace osg;

ColorMatrix::ColorMatrix()
{
}


ColorMatrix::~ColorMatrix()
{
}

void ColorMatrix::apply(State& state) const
{
#if defined(OSG_GL_FIXED_FUNCTION_AVAILABLE) && !defined(OSG_GLES1_AVAILABLE)
    unsigned int contextID = state.getContextID();

    static bool s_ARB_imaging = isGLExtensionSupported(contextID,"GL_ARB_imaging");
    if (s_ARB_imaging)
    {
        glMatrixMode( GL_COLOR );
        glLoadMatrix(_matrix.ptr());
        glMatrixMode( GL_MODELVIEW );
    }
#else
    OSG_NOTICE<<"Warning: ColorMatrix::apply(State&) - not supported."<<std::endl;
#endif
}
