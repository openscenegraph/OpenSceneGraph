/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2003 Robert Osfield 
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
// Ideas and code borrowed from GLUT pointburst demo
// written by Mark J. Kilgard

#include <osg/GLExtensions>
#include <osg/GL>
#include <osg/Point>
#include <osg/Notify>

using namespace osg;

// if extensions not defined by gl.h (or via glext.h) define them
// ourselves and allow the extensions to detectd at runtine using
// osg::isGLExtensionSupported().
#if defined(GL_SGIS_point_parameters) && !defined(GL_EXT_point_parameters)
/* Use the EXT point parameters interface for the SGIS implementation. */
#  define GL_POINT_SIZE_MIN_EXT GL_POINT_SIZE_MIN_SGIS
#  define GL_POINT_SIZE_MAX_EXT GL_POINT_SIZE_MAX_SGIS
#  define GL_POINT_FADE_THRESHOLD_SIZE_EXT GL_POINT_FADE_THRESHOLD_SIZE_SGIS
#  define GL_DISTANCE_ATTENUATION_EXT GL_DISTANCE_ATTENUATION_SGIS
#  define GL_EXT_point_parameters 1
#endif

#if !defined(GL_EXT_point_parameters)
#  define GL_POINT_SIZE_MIN_EXT               0x8126
#  define GL_POINT_SIZE_MAX_EXT               0x8127
#  define GL_POINT_FADE_THRESHOLD_SIZE_EXT    0x8128
#  define GL_DISTANCE_ATTENUATION_EXT         0x8129
#  define GL_EXT_point_parameters 1
#endif

#ifndef PFNGLPOINTPARAMETERFEXTPROC
typedef void (APIENTRY * PFNGLPOINTPARAMETERFEXTPROC) (GLenum pname, GLfloat param);
#endif
#ifndef PFNGLPOINTPARAMETERFVEXTPROC
typedef void (APIENTRY * PFNGLPOINTPARAMETERFVEXTPROC) (GLenum pname, const GLfloat *params);
#endif

PFNGLPOINTPARAMETERFEXTPROC s_PointParameterfEXT = NULL;
PFNGLPOINTPARAMETERFVEXTPROC s_PointParameterfvEXT = NULL;

Point::Point()
{
    _size = 1.0f;                // TODO find proper default
    _fadeThresholdSize = 1.0f;   // TODO find proper default
                                 // TODO find proper default
    _distanceAttenuation = Vec3(0.0f, 1.0f/5.f, 0.0f);
}


Point::~Point()
{
}

void Point::init_GL_EXT()
{
    if (isGLExtensionSupported("GL_EXT_point_parameters"))
    {
        s_PointParameterfEXT = (PFNGLPOINTPARAMETERFEXTPROC)
            getGLExtensionFuncPtr("glPointParameterfEXT");
        s_PointParameterfvEXT = (PFNGLPOINTPARAMETERFVEXTPROC)
            getGLExtensionFuncPtr("glPointParameterfvEXT");
    }
    else if (isGLExtensionSupported("GL_SGIS_point_parameters"))
    {
        s_PointParameterfEXT = (PFNGLPOINTPARAMETERFEXTPROC)
            getGLExtensionFuncPtr("glPointParameterfSGIS");
        s_PointParameterfvEXT = (PFNGLPOINTPARAMETERFVEXTPROC)
            getGLExtensionFuncPtr("glPointParameterfvSGIS");
    }

}

void Point::setSize( float size )
{
    _size = size;
}


void Point::setFadeThresholdSize(float fadeThresholdSize)
{
    _fadeThresholdSize = fadeThresholdSize;
}


void Point::setDistanceAttenuation(const Vec3& distanceAttenuation)
{
    _distanceAttenuation = distanceAttenuation;
}


void Point::apply(State&) const
{
    glPointSize(_size);

    static bool s_gl_ext_init=false;

    if (!s_gl_ext_init)
    {
        s_gl_ext_init = true;
        init_GL_EXT();
    }

    if (s_PointParameterfvEXT) s_PointParameterfvEXT(GL_DISTANCE_ATTENUATION_EXT, (const GLfloat*)&_distanceAttenuation);
    if (s_PointParameterfEXT) s_PointParameterfEXT(GL_POINT_FADE_THRESHOLD_SIZE_EXT, _fadeThresholdSize);

}

