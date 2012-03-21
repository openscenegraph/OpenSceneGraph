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
    const unsigned int contextID = state.getContextID();

    const Extensions* extensions = getExtensions(contextID,true);

    if (!extensions->isClampColorSupported())
    {
        OSG_WARN<<"Warning: ClampColor::apply(..) failed, ClampColor is not support by OpenGL driver."<<std::endl;
        return;
    }

    extensions->glClampColor(GL_CLAMP_VERTEX_COLOR, _clampVertexColor);
    extensions->glClampColor(GL_CLAMP_FRAGMENT_COLOR, _clampFragmentColor);
    extensions->glClampColor(GL_CLAMP_READ_COLOR, _clampReadColor);
}


typedef buffered_value< ref_ptr<ClampColor::Extensions> > BufferedExtensions;
static BufferedExtensions s_extensions;

ClampColor::Extensions* ClampColor::getExtensions(unsigned int contextID,bool createIfNotInitalized)
{
    if (!s_extensions[contextID] && createIfNotInitalized) s_extensions[contextID] = new Extensions(contextID);
    return s_extensions[contextID].get();
}

void ClampColor::setExtensions(unsigned int contextID,Extensions* extensions)
{
    s_extensions[contextID] = extensions;
}


ClampColor::Extensions::Extensions(unsigned int contextID)
{
    setupGLExtensions(contextID);
}

ClampColor::Extensions::Extensions(const Extensions& rhs):
    Referenced()
{
    _isClampColorSupported = rhs._isClampColorSupported;
    _glClampColor = rhs._glClampColor;
}

void ClampColor::Extensions::lowestCommonDenominator(const Extensions& rhs)
{
    if (!rhs._isClampColorSupported) _isClampColorSupported = false;
    if (!rhs._glClampColor)          _glClampColor = 0;
}

void ClampColor::Extensions::setupGLExtensions(unsigned int contextID)
{
    _isClampColorSupported = OSG_GL3_FEATURES ||
                             isGLExtensionSupported(contextID,"GL_ARB_color_buffer_float") ||
                             strncmp((const char*)glGetString(GL_VERSION),"2.0",3)>=0;

    setGLExtensionFuncPtr(_glClampColor, "glClampColor", "glClampColorARB");
}

void ClampColor::Extensions::glClampColor(GLenum target, GLenum mode) const
{
    if (_glClampColor)
    {
        _glClampColor(target,mode);
    }
    else
    {
        OSG_WARN<<"Error: glClampColor not supported by OpenGL driver"<<std::endl;
    }
}


