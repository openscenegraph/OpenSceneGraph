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
#include <osg/BlendEquation>
#include <osg/GLExtensions>
#include <osg/State>
#include <osg/Notify>
#include <osg/buffered_value>


using namespace osg;

BlendEquation::BlendEquation():
    _equation(FUNC_ADD_EXT)
{
}

BlendEquation::BlendEquation(GLenum equation):
    _equation(equation)
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
        notify(WARN)<<"Warning: BlendEquation::apply(..) failed, BlendEquation is not support by OpenGL driver."<<std::endl;
        return;
    }

    extensions->glBlendEquation(_equation);
    
}


typedef buffered_value< ref_ptr<BlendEquation::Extensions> > BufferedExtensions;
static BufferedExtensions s_extensions;

BlendEquation::Extensions* BlendEquation::getExtensions(unsigned int contextID,bool createIfNotInitalized)
{
    if (!s_extensions[contextID] && createIfNotInitalized) s_extensions[contextID] = new Extensions;
    return s_extensions[contextID].get();
}

void BlendEquation::setExtensions(unsigned int contextID,Extensions* extensions)
{
    s_extensions[contextID] = extensions;
}


BlendEquation::Extensions::Extensions()
{
    setupGLExtenions();
}

BlendEquation::Extensions::Extensions(const Extensions& rhs):
    Referenced()
{
    _isBlendEquationSupported = rhs._isBlendEquationSupported;
}

void BlendEquation::Extensions::lowestCommonDenominator(const Extensions& rhs)
{
    if (!rhs._isBlendEquationSupported)  _isBlendEquationSupported = false;
    if (!rhs._glBlendEquation)           _glBlendEquation = 0;
}

void BlendEquation::Extensions::setupGLExtenions()
{
    _isBlendEquationSupported = isGLExtensionSupported("GL_EXT_blend_equation") ||
                             strncmp((const char*)glGetString(GL_VERSION),"1.2",3)>=0;

    _glBlendEquation = getGLExtensionFuncPtr("glBlendEquation", "glBlendEquationEXT");
}

void BlendEquation::Extensions::glBlendEquation(GLenum mode) const
{
    if (_glBlendEquation)
    {
        typedef void (APIENTRY * GLBlendEquationProc) (GLenum mode);
        ((GLBlendEquationProc)_glBlendEquation)(mode);
    }
    else
    {
        notify(WARN)<<"Error: glBlendEquation not supported by OpenGL driver"<<std::endl;
    }
}


