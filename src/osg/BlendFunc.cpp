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
#include <osg/BlendFunc>
#include <osg/GLExtensions>
#include <osg/State>
#include <osg/Notify>

using namespace osg;

BlendFunc::BlendFunc():
    _source_factor(SRC_ALPHA),
    _destination_factor(ONE_MINUS_SRC_ALPHA),
    _source_factor_alpha(SRC_ALPHA),
    _destination_factor_alpha(ONE_MINUS_SRC_ALPHA)
{
}

BlendFunc::BlendFunc(GLenum source, GLenum destination):
    _source_factor(source),
    _destination_factor(destination),
    _source_factor_alpha(source),
    _destination_factor_alpha(destination)
{
}

BlendFunc::BlendFunc(GLenum source, GLenum destination, GLenum source_alpha, GLenum destination_alpha):
    _source_factor(source),
    _destination_factor(destination),
    _source_factor_alpha(source_alpha),
    _destination_factor_alpha(destination_alpha)
{
}

BlendFunc::~BlendFunc()
{
}

void BlendFunc::apply(State& state) const
{
    if (_source_factor != _source_factor_alpha ||
        _destination_factor != _destination_factor_alpha)
    {
        // get the contextID (user defined ID of 0 upwards) for the
        // current OpenGL context.
        const unsigned int contextID = state.getContextID();

        const Extensions* extensions = getExtensions(contextID,true);

        if (!extensions->isBlendFuncSeparateSupported())
        {
            OSG_WARN<<"Warning: BlendFunc::apply(..) failed, BlendFuncSeparate is not support by OpenGL driver, falling back to BlendFunc."<<std::endl;
        }
        else
        {
            extensions->glBlendFuncSeparate(_source_factor, _destination_factor, _source_factor_alpha, _destination_factor_alpha);
            return;
        }
    }

    glBlendFunc( _source_factor, _destination_factor );
}


typedef buffered_value< ref_ptr<BlendFunc::Extensions> > BufferedExtensions;
static BufferedExtensions s_extensions;

BlendFunc::Extensions* BlendFunc::getExtensions(unsigned int contextID,bool createIfNotInitalized)
{
    if (!s_extensions[contextID] && createIfNotInitalized) s_extensions[contextID] = new Extensions(contextID);
    return s_extensions[contextID].get();
}

void BlendFunc::setExtensions(unsigned int contextID,Extensions* extensions)
{
    s_extensions[contextID] = extensions;
}


BlendFunc::Extensions::Extensions(unsigned int contextID)
{
    setupGLExtensions(contextID);
}

BlendFunc::Extensions::Extensions(const Extensions& rhs):
    Referenced()
{
    _isBlendFuncSeparateSupported = rhs._isBlendFuncSeparateSupported;
    _glBlendFuncSeparate = rhs._glBlendFuncSeparate;
}

void BlendFunc::Extensions::lowestCommonDenominator(const Extensions& rhs)
{
    if (!rhs._isBlendFuncSeparateSupported)  _isBlendFuncSeparateSupported = false;
    if (!rhs._glBlendFuncSeparate)           _glBlendFuncSeparate = 0;
}

void BlendFunc::Extensions::setupGLExtensions(unsigned int contextID)
{
    _isBlendFuncSeparateSupported = OSG_GLES2_FEATURES || OSG_GL3_FEATURES ||
                                    isGLExtensionSupported(contextID, "GL_EXT_blend_func_separate") ||
                                    strncmp((const char*)glGetString(GL_VERSION), "1.4", 3) >= 0;

     setGLExtensionFuncPtr(_glBlendFuncSeparate, "glBlendFuncSeparate", "glBlendFuncSeparateEXT");
}

void BlendFunc::Extensions::glBlendFuncSeparate(GLenum sfactorRGB,
                                                GLenum dfactorRGB,
                                                GLenum sfactorAlpha,
                                                GLenum dfactorAlpha) const
{
    if (_glBlendFuncSeparate)
    {
        _glBlendFuncSeparate(sfactorRGB, dfactorRGB, sfactorAlpha, dfactorAlpha);
    }
    else
    {
        OSG_WARN<<"Error: glBlendFuncSeparate not supported by OpenGL driver"<<std::endl;
    }
}
