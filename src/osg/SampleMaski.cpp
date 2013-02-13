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

#include <osg/SampleMaski>
#include <osg/State>
#include <osg/GLExtensions>
#include <osg/Notify>

using namespace osg;


SampleMaski::SampleMaski()
{
    _sampleMask[0u] = ~0u;
    _sampleMask[1u] = ~0u;
}

SampleMaski::SampleMaski(const SampleMaski& sampleMaski,const CopyOp& copyop):
    StateAttribute(sampleMaski,copyop)
{
    _sampleMask[0u] = sampleMaski._sampleMask[0u];
    _sampleMask[1u] = sampleMaski._sampleMask[1u];
}

SampleMaski::~SampleMaski()
{
}

int SampleMaski::compare(const StateAttribute& sa) const
{
    // check the types are equal and then create the rhs variable
    // used by the COMPARE_StateAttribute_Parameter macros below.
    COMPARE_StateAttribute_Types(SampleMaski,sa)

    COMPARE_StateAttribute_Parameter(_sampleMask[0u])
    COMPARE_StateAttribute_Parameter(_sampleMask[1u])

    return 0; // passed all the above comparison macros, must be equal.
}

void SampleMaski::apply(State& state) const
{
    // get "per-context" extensions
    const unsigned int contextID = state.getContextID();
    const Extensions* extensions = getExtensions(contextID,true);
    
    if ( (extensions->isTextureMultisampleSupported()) || (extensions->isOpenGL32upported()) || (extensions->isSampleMaskiSupported())  )
    {
        extensions->glSampleMaski(0u, _sampleMask[0u]);
//For now we use only 32-bit Sample mask
//        extensions->glSampleMaski(1u, _sampleMask[1u]);
        return;
    }

    OSG_WARN << "SampleMaski failed as the required graphics capabilities were\n"
                "   not found (contextID " << contextID << "). OpenGL 3.2 or \n"
                "   ARB_texture_multisample extension is required." << std::endl;
}


typedef buffered_value< ref_ptr<SampleMaski::Extensions> > BufferedExtensions;
static BufferedExtensions s_extensions;

SampleMaski::Extensions* SampleMaski::getExtensions(unsigned int contextID,bool createIfNotInitalized)
{
    if (!s_extensions[contextID] && createIfNotInitalized) s_extensions[contextID] = new Extensions(contextID);
    return s_extensions[contextID].get();
}

void SampleMaski::setExtensions(unsigned int contextID,Extensions* extensions)
{
    s_extensions[contextID] = extensions;
}

SampleMaski::Extensions::Extensions(unsigned int contextID)
{
    setupGLExtensions(contextID);
}

SampleMaski::Extensions::Extensions(const Extensions& rhs):
    Referenced()
{
    _isTextureMultisampleSupported = rhs._isTextureMultisampleSupported;
    _isOpenGL32upported = rhs._isOpenGL32upported;
    _isSampleMaskiSupported = rhs._isSampleMaskiSupported;
    _glSampleMaski = rhs._glSampleMaski;
}


void SampleMaski::Extensions::lowestCommonDenominator(const Extensions& rhs)
{
    if (!rhs._isTextureMultisampleSupported) _isTextureMultisampleSupported = false;
    if (!rhs._isOpenGL32upported) _isOpenGL32upported = false;
    if (!rhs._isSampleMaskiSupported) _isSampleMaskiSupported = false;

    if (!rhs._glSampleMaski) _glSampleMaski = NULL;
}

void SampleMaski::Extensions::setupGLExtensions(unsigned int contextID)
{
    // extension support
    _isTextureMultisampleSupported = isGLExtensionSupported(contextID, "GL_ARB_texture_multisample");
    _isOpenGL32upported = getGLVersionNumber() >= 3.2;

    // function pointers
    setGLExtensionFuncPtr(_glSampleMaski, "glSampleMaski");


    // protect against buggy drivers (maybe not necessary)
    if (!_glSampleMaski) _isSampleMaskiSupported = false;

    // notify
    if( _isOpenGL32upported )
    {
       OSG_INFO << "SampleMaski is going to use OpenGL 3.2 API (contextID " << contextID << ")." << std::endl;
    }
    else if( _isTextureMultisampleSupported )
    {
       OSG_INFO << "SampleMaski is going to use GL_ARB_texture_multisample extension (contextID " << contextID << ")." << std::endl;
    }
    else
    {
       OSG_INFO << "SampleMaski did not found required graphics capabilities\n"
                   "   (contextID " << contextID << "). OpenGL 3.2 or \n"
                   "   GL_ARB_texture_multisample extension is required." << std::endl;
    }
}

void SampleMaski::Extensions::glSampleMaski(GLuint maskNumber, GLbitfield mask) const
{
    _glSampleMaski(maskNumber, mask);
}

