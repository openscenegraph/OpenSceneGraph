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
#include <osg/State>
#include <osg/Fog>
#include <osg/Notify>

using namespace osg;

#ifndef GL_FOG_COORDINATE_SOURCE
    #define GL_FOG_COORDINATE_SOURCE    0x8450
#endif

Fog::Fog()
{
    _mode = EXP;
    _density = 1.0f;
    _start   = 0.0f;
    _end     = 1.0f;
    _color.set( 0.0f, 0.0f, 0.0f, 0.0f);
    _fogCoordinateSource = FRAGMENT_DEPTH;
    _useRadialFog = false;
}


Fog::~Fog()
{
}

void Fog::apply(State& state) const
{
#ifdef OSG_GL_FIXED_FUNCTION_AVAILABLE

#ifdef OSG_GLES1_AVAILABLE
    #define glFogi glFogx
#endif

    glFogi( GL_FOG_MODE,     _mode );

    glFogf( GL_FOG_DENSITY,  _density );
    glFogf( GL_FOG_START,    _start );
    glFogf( GL_FOG_END,      _end );
    glFogfv( GL_FOG_COLOR,    (GLfloat*)_color.ptr() );

    static bool fogCoordExtensionSupported = osg::isGLExtensionSupported(state.getContextID(),"GL_EXT_fog_coord");
    if (fogCoordExtensionSupported)
    {
        glFogi(GL_FOG_COORDINATE_SOURCE,_fogCoordinateSource);
    }

    static bool fogDistanceExtensionSupported = osg::isGLExtensionSupported(state.getContextID(),"GL_NV_fog_distance");
    if (fogDistanceExtensionSupported)
    {
        if(_useRadialFog)
            glFogf(GL_FOG_DISTANCE_MODE_NV, GL_EYE_RADIAL_NV);
        else
            glFogf(GL_FOG_DISTANCE_MODE_NV, GL_EYE_PLANE_ABSOLUTE_NV);
    }

#else
    OSG_NOTICE<<"Warning: Fog::apply(State&) - not supported."<<std::endl;
#endif
}
