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
#include <osg/BlendEquation>
#include <osg/GLExtensions>
#include <osg/State>
#include <osg/Notify>
#include <osg/buffered_value>


using namespace osg;

BlendEquation::BlendEquation():
    _equationRGB(FUNC_ADD),
    _equationAlpha(FUNC_ADD)
{
}

BlendEquation::BlendEquation(Equation equation):
    _equationRGB(equation),
    _equationAlpha(equation)
{
}

BlendEquation::BlendEquation(Equation equationRGB, Equation equationAlpha):
    _equationRGB(equationRGB),
    _equationAlpha(equationAlpha)
{
}

BlendEquation::~BlendEquation()
{
}

void BlendEquation::apply(State& state) const
{

   // get the contextID (user defined ID of 0 upwards) for the
    // current OpenGL context.
    const unsigned int contextID = state.getContextID();

    const Extensions* extensions = getExtensions(contextID,true);

    if (!extensions->isBlendEquationSupported())
    {
        OSG_WARN<<"Warning: BlendEquation::apply(..) failed, BlendEquation is not support by OpenGL driver."<<std::endl;
        return;
    }

    if((_equationRGB == ALPHA_MIN || _equationRGB == ALPHA_MAX) && !extensions->isSGIXMinMaxSupported())
    {
        OSG_WARN<<"Warning: BlendEquation::apply(..) failed, SGIX_blend_alpha_minmax extension is not supported by OpenGL driver." << std::endl;
        return;
    }

    if(_equationRGB == LOGIC_OP && !extensions->isLogicOpSupported())
    {
        OSG_WARN<<"Warning: BlendEquation::apply(..) failed, EXT_blend_logic_op extension is not supported by OpenGL driver." << std::endl;
        return;
    }

    if (_equationRGB == _equationAlpha)
        extensions->glBlendEquation(static_cast<GLenum>(_equationRGB));
    else
    {
        if (extensions->isBlendEquationSeparateSupported())
            extensions->glBlendEquationSeparate(static_cast<GLenum>(_equationRGB), static_cast<GLenum>(_equationAlpha));
        else
        {
            OSG_WARN<<"Warning: BlendEquation::apply(..) failed, EXT_blend_equation_separate extension is not supported by OpenGL driver." << std::endl;
            return;
        }
    }
}


typedef buffered_value< ref_ptr<BlendEquation::Extensions> > BufferedExtensions;
static BufferedExtensions s_extensions;

BlendEquation::Extensions* BlendEquation::getExtensions(unsigned int contextID,bool createIfNotInitalized)
{
    if (!s_extensions[contextID] && createIfNotInitalized) s_extensions[contextID] = new Extensions(contextID);
    return s_extensions[contextID].get();
}

void BlendEquation::setExtensions(unsigned int contextID,Extensions* extensions)
{
    s_extensions[contextID] = extensions;
}


BlendEquation::Extensions::Extensions(unsigned int contextID)
{
    setupGLExtensions(contextID);
}

BlendEquation::Extensions::Extensions(const Extensions& rhs):
    Referenced()
{
    _isBlendEquationSupported = rhs._isBlendEquationSupported;
    _isBlendEquationSeparateSupported = rhs._isBlendEquationSeparateSupported;
    _isSGIXMinMaxSupported = rhs._isSGIXMinMaxSupported;
    _isLogicOpSupported = rhs._isLogicOpSupported;
    _glBlendEquation = rhs._glBlendEquation;
    _glBlendEquationSeparate = rhs._glBlendEquationSeparate;
}

void BlendEquation::Extensions::lowestCommonDenominator(const Extensions& rhs)
{
    if (!rhs._isBlendEquationSupported)  _isBlendEquationSupported = false;
    if (!rhs._glBlendEquation)           _glBlendEquation = 0;
    if (!rhs._isBlendEquationSeparateSupported)  _isBlendEquationSeparateSupported = false;
    if (!rhs._glBlendEquationSeparate)           _glBlendEquationSeparate = 0;
}

void BlendEquation::Extensions::setupGLExtensions(unsigned int contextID)
{
    bool bultInSupport = OSG_GLES2_FEATURES || OSG_GL3_FEATURES;
    _isBlendEquationSupported = bultInSupport ||
        isGLExtensionSupported(contextID, "GL_EXT_blend_equation") ||
        strncmp((const char*)glGetString(GL_VERSION), "1.2", 3) >= 0;

    _isBlendEquationSeparateSupported = bultInSupport ||
        isGLExtensionSupported(contextID, "GL_EXT_blend_equation_separate") ||
        strncmp((const char*)glGetString(GL_VERSION), "2.0", 3) >= 0;

    _isSGIXMinMaxSupported = isGLExtensionSupported(contextID, "GL_SGIX_blend_alpha_minmax");
    _isLogicOpSupported = isGLExtensionSupported(contextID, "GL_EXT_blend_logic_op");

    setGLExtensionFuncPtr(_glBlendEquation, "glBlendEquation", "glBlendEquationEXT");
    setGLExtensionFuncPtr(_glBlendEquationSeparate, "glBlendEquationSeparate", "glBlendEquationSeparateEXT");
}

void BlendEquation::Extensions::glBlendEquation(GLenum mode) const
{
    if (_glBlendEquation)
    {
        _glBlendEquation(mode);
    }
    else
    {
        OSG_WARN<<"Error: glBlendEquation not supported by OpenGL driver"<<std::endl;
    }
}

void BlendEquation::Extensions::glBlendEquationSeparate(GLenum modeRGB, GLenum modeAlpha) const
{
    if (_glBlendEquationSeparate)
    {
        _glBlendEquationSeparate(modeRGB, modeAlpha);
    }
    else
    {
        OSG_WARN<<"Error: glBlendEquationSeparate not supported by OpenGL driver"<<std::endl;
    }
}

