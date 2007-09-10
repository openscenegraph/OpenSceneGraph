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
#include <osg/Multisample>
#include <osg/State>
#include <osg/Notify>
#include <osg/buffered_value>

using namespace osg;
 

Multisample::Multisample() : _mode(DONT_CARE)
{
    _coverage = 1;
    _invert   = false;
}

Multisample::~Multisample()
{
}

void Multisample::apply(State& state) const
{
    // get the contextID (user defined ID of 0 upwards) for the 
    // current OpenGL context.
    const unsigned int contextID = state.getContextID();
    
    const Extensions* extensions = getExtensions(contextID,true);
                                        
    if (!extensions->isMultisampleSupported())
    {
        notify(WARN)<<"Warning: Multisample::apply(..) failed, Multisample is not support by OpenGL driver."<<std::endl;
        return;
    }

    if(extensions->isMultisampleFilterHintSupported())
        glHint(GL_MULTISAMPLE_FILTER_HINT_NV, _mode);
    extensions->glSampleCoverage(_coverage, _invert);
}





typedef buffered_value< ref_ptr<Multisample::Extensions> > BufferedExtensions;
static BufferedExtensions s_extensions;

Multisample::Extensions* Multisample::getExtensions(unsigned int contextID,bool createIfNotInitalized)
{
    if (!s_extensions[contextID] && createIfNotInitalized) s_extensions[contextID] = new Extensions(contextID);
    return s_extensions[contextID].get();
}

void Multisample::setExtensions(unsigned int contextID,Extensions* extensions)
{
    s_extensions[contextID] = extensions;
}


Multisample::Extensions::Extensions(unsigned int contextID)
{
    setupGLExtensions(contextID);
}

Multisample::Extensions::Extensions(const Extensions& rhs):
    Referenced()
{
    _isMultisampleSupported = rhs._isMultisampleSupported;
    _isMultisampleFilterHintSupported = rhs._isMultisampleFilterHintSupported;
}

void Multisample::Extensions::lowestCommonDenominator(const Extensions& rhs)
{
    if (!rhs._isMultisampleSupported)  _isMultisampleSupported = false;
    if (!rhs._isMultisampleFilterHintSupported)  _isMultisampleFilterHintSupported = false;
    if (!rhs._glSampleCoverage)        _glSampleCoverage = 0;
}

void Multisample::Extensions::setupGLExtensions(unsigned int contextID)
{
    _isMultisampleSupported = isGLExtensionSupported(contextID,"GL_ARB_multisample");
    _isMultisampleFilterHintSupported = isGLExtensionSupported(contextID,"GL_NV_multisample_filter_hint");

    setGLExtensionFuncPtr(_glSampleCoverage, "glSampleCoverageARB");
}

void Multisample::Extensions::glSampleCoverage(GLclampf value, GLboolean invert) const
{
    if (_glSampleCoverage)
    {
        _glSampleCoverage(value, invert);
    }
    else
    {
        notify(WARN)<<"Error: glSampleCoverage not supported by OpenGL driver"<<std::endl;
    }
}

