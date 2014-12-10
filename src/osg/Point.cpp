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
#include <osg/GL>
#include <osg/State>
#include <osg/Point>
#include <osg/Notify>

using namespace osg;

// ARB, EXT and SGIS versions use same values as OpenGL version 1.4.
#ifndef GL_VERSION_1_4
#  define GL_POINT_SIZE_MIN                   0x8126
#  define GL_POINT_SIZE_MAX                   0x8127
#  define GL_POINT_FADE_THRESHOLD_SIZE        0x8128
#  define GL_POINT_DISTANCE_ATTENUATION       0x8129
#endif

Point::Point()
{
    _size = 1.0f;                               // TODO find proper default
    _fadeThresholdSize = 1.0f;                  // TODO find proper default
    _distanceAttenuation = Vec3(1, 0.0, 0.0);   // TODO find proper default

    _minSize = 0.0;
    _maxSize = 100.0;//depends on mulitsampling ... some default necessary
}

Point::Point(float size)
{
    _size = size;
    _fadeThresholdSize = 1.0f;                  // TODO find proper default
    _distanceAttenuation = Vec3(1, 0.0, 0.0);   // TODO find proper default

    _minSize = 0.0;
    _maxSize = 100.0;//depends on mulitsampling ... some default necessary
}

Point::~Point()
{
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

void Point::setMinSize(float minSize)
{
    _minSize = minSize;
}

void Point::setMaxSize(float maxSize)
{
    _maxSize = maxSize;
}

void Point::apply(State& state) const
{
#ifdef OSG_GL_FIXED_FUNCTION_AVAILABLE
    glPointSize(_size);

    const GLExtensions* extensions = state.get<GLExtensions>();

    if (!extensions->isPointParametersSupported)
        return;

    extensions->glPointParameterfv(GL_POINT_DISTANCE_ATTENUATION, (const GLfloat*)&_distanceAttenuation);
    extensions->glPointParameterf(GL_POINT_FADE_THRESHOLD_SIZE, _fadeThresholdSize);
    extensions->glPointParameterf(GL_POINT_SIZE_MIN, _minSize);
    extensions->glPointParameterf(GL_POINT_SIZE_MAX, _maxSize);
#else
    OSG_NOTICE<<"Warning: Point::apply(State&) - not supported."<<std::endl;
#endif
}
