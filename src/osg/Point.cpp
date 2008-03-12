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
    glPointSize(_size);

    const unsigned int contextID = state.getContextID();
    const Extensions* extensions = getExtensions(contextID,true);

    if (!extensions->isPointParametersSupported())
        return;

    extensions->glPointParameterfv(GL_POINT_DISTANCE_ATTENUATION, (const GLfloat*)&_distanceAttenuation);
    extensions->glPointParameterf(GL_POINT_FADE_THRESHOLD_SIZE, _fadeThresholdSize);
    extensions->glPointParameterf(GL_POINT_SIZE_MIN, _minSize);
    extensions->glPointParameterf(GL_POINT_SIZE_MAX, _maxSize);
}


typedef buffered_value< ref_ptr<Point::Extensions> > BufferedExtensions;
static BufferedExtensions s_extensions;

Point::Extensions* Point::getExtensions(unsigned int contextID,bool createIfNotInitalized)
{
    if (!s_extensions[contextID] && createIfNotInitalized) s_extensions[contextID] = new Extensions(contextID);
    return s_extensions[contextID].get();
}

void Point::setExtensions(unsigned int contextID,Extensions* extensions)
{
    s_extensions[contextID] = extensions;
}

Point::Extensions::Extensions(unsigned int contextID)
{
    setupGLExtensions(contextID);
}

Point::Extensions::Extensions(const Extensions& rhs):
    Referenced()
{
    _isPointParametersSupported = rhs._isPointParametersSupported;
    _isPointSpriteCoordOriginSupported = rhs._isPointSpriteCoordOriginSupported;
    _glPointParameteri = rhs._glPointParameteri;
    _glPointParameterf = rhs._glPointParameterf;
    _glPointParameterfv = rhs._glPointParameterfv;
}

void Point::Extensions::lowestCommonDenominator(const Extensions& rhs)
{
    if (!rhs._isPointParametersSupported)  _isPointParametersSupported = false;
    if (!rhs._isPointSpriteCoordOriginSupported)  _isPointSpriteCoordOriginSupported = false;
    if (!rhs._glPointParameteri)           _glPointParameteri = 0;
    if (!rhs._glPointParameterf)           _glPointParameterf = 0;
    if (!rhs._glPointParameterfv)          _glPointParameterfv = 0;
}

void Point::Extensions::setupGLExtensions(unsigned int contextID)
{
    _isPointParametersSupported = strncmp((const char*)glGetString(GL_VERSION),"1.4",3)>=0 ||
                                  isGLExtensionSupported(contextID,"GL_ARB_point_parameters") ||
                                  isGLExtensionSupported(contextID,"GL_EXT_point_parameters") ||
                                  isGLExtensionSupported(contextID,"GL_SGIS_point_parameters");
            
    _isPointSpriteCoordOriginSupported = strncmp((const char*)glGetString(GL_VERSION),"2.0",3)>=0;

    setGLExtensionFuncPtr(_glPointParameteri, "glPointParameteri", "glPointParameteriARB");
    if (!_glPointParameteri) setGLExtensionFuncPtr(_glPointParameteri, "glPointParameteriEXT", "glPointParameteriSGIS");

    setGLExtensionFuncPtr(_glPointParameterf, "glPointParameterf", "glPointParameterfARB");
    if (!_glPointParameterf) setGLExtensionFuncPtr(_glPointParameterf, "glPointParameterfEXT", "glPointParameterfSGIS");

    setGLExtensionFuncPtr(_glPointParameterfv, "glPointParameterfv", "glPointParameterfvARB");
    if (!_glPointParameterfv) setGLExtensionFuncPtr(_glPointParameterfv, "glPointParameterfvEXT", "glPointParameterfvSGIS");
}

void Point::Extensions::glPointParameteri(GLenum pname, GLint param) const
{
    if (_glPointParameteri)
    {
        _glPointParameteri(pname, param);
    }
    else
    {
        notify(WARN)<<"Error: glPointParameteri not supported by OpenGL driver"<<std::endl;
    }
}

void Point::Extensions::glPointParameterf(GLenum pname, GLfloat param) const
{
    if (_glPointParameterf)
    {
        _glPointParameterf(pname, param);
    }
    else
    {
        notify(WARN)<<"Error: glPointParameterf not supported by OpenGL driver"<<std::endl;
    }
}

void Point::Extensions::glPointParameterfv(GLenum pname, const GLfloat *params) const
{
    if (_glPointParameterfv)
    {
        _glPointParameterfv(pname, params);
    }
    else
    {
        notify(WARN)<<"Error: glPointParameterfv not supported by OpenGL driver"<<std::endl;
    }
}
