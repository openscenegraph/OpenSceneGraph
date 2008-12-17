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

#include <osg/StencilTwoSided>
#include <osg/State>
#include <osg/GLExtensions>
#include <osg/Notify>

using namespace osg;

StencilTwoSided::StencilTwoSided()
{
    // set up same defaults as glStencilFunc.
    _func[FRONT] = _func[BACK] = ALWAYS;
    _funcRef[FRONT] = _funcRef[BACK] = 0;
    _funcMask[FRONT] = _funcMask[BACK] = ~0u;
        
    // set up same defaults as glStencilOp.
    _sfail[FRONT] = _sfail[BACK] = KEEP;
    _zfail[FRONT] = _zfail[BACK] = KEEP;
    _zpass[FRONT] = _zpass[BACK] = KEEP;

    _writeMask[FRONT] = _writeMask[BACK] = ~0u;
}

StencilTwoSided::StencilTwoSided(const StencilTwoSided& stencil,const CopyOp& copyop):
    StateAttribute(stencil,copyop)
{
    _func[FRONT] = stencil._func[FRONT];
    _funcRef[FRONT] = stencil._funcRef[FRONT];
    _funcMask[FRONT] = stencil._funcMask[FRONT];
    _sfail[FRONT] = stencil._sfail[FRONT];
    _zfail[FRONT] = stencil._zfail[FRONT];
    _zpass[FRONT] = stencil._zpass[FRONT];
    _writeMask[FRONT] = stencil._writeMask[FRONT];

    _func[BACK] = stencil._func[BACK];
    _funcRef[BACK] = stencil._funcRef[BACK];
    _funcMask[BACK] = stencil._funcMask[BACK];
    _sfail[BACK] = stencil._sfail[BACK];
    _zfail[BACK] = stencil._zfail[BACK];
    _zpass[BACK] = stencil._zpass[BACK];
    _writeMask[BACK] = stencil._writeMask[BACK];
}

StencilTwoSided::~StencilTwoSided()
{
}

int StencilTwoSided::compare(const StateAttribute& sa) const
{
    // check the types are equal and then create the rhs variable
    // used by the COMPARE_StateAttribute_Parameter macro's below.
    COMPARE_StateAttribute_Types(StencilTwoSided,sa)

    // compare each parameter in turn against the rhs.
    COMPARE_StateAttribute_Parameter(_func[FRONT])
    COMPARE_StateAttribute_Parameter(_funcRef[FRONT])
    COMPARE_StateAttribute_Parameter(_funcMask[FRONT])
    COMPARE_StateAttribute_Parameter(_sfail[FRONT])
    COMPARE_StateAttribute_Parameter(_zfail[FRONT])
    COMPARE_StateAttribute_Parameter(_zpass[FRONT])
    COMPARE_StateAttribute_Parameter(_writeMask[FRONT])

    COMPARE_StateAttribute_Parameter(_func[BACK])
    COMPARE_StateAttribute_Parameter(_funcRef[BACK])
    COMPARE_StateAttribute_Parameter(_funcMask[BACK])
    COMPARE_StateAttribute_Parameter(_sfail[BACK])
    COMPARE_StateAttribute_Parameter(_zfail[BACK])
    COMPARE_StateAttribute_Parameter(_zpass[BACK])
    COMPARE_StateAttribute_Parameter(_writeMask[BACK])

    return 0; // passed all the above comparison macro's, must be equal.
}

void StencilTwoSided::apply(State& state) const
{
    const unsigned int contextID = state.getContextID();
    const Extensions* extensions = getExtensions(contextID,true);

    if (!extensions->isStencilTwoSidedSupported())
        return;
        
    extensions->glActiveStencilFace(GL_BACK);
    glStencilOp((GLenum)_sfail[BACK],(GLenum)_zfail[BACK],(GLenum)_zpass[BACK]);
    glStencilMask(_writeMask[BACK]);
    glStencilFunc((GLenum)_func[BACK],_funcRef[BACK],_funcMask[BACK]);

    extensions->glActiveStencilFace(GL_FRONT);
    glStencilOp((GLenum)_sfail[FRONT],(GLenum)_zfail[FRONT],(GLenum)_zpass[FRONT]);
    glStencilMask(_writeMask[FRONT]);
    glStencilFunc((GLenum)_func[FRONT],_funcRef[FRONT],_funcMask[FRONT]);
}


typedef buffered_value< ref_ptr<StencilTwoSided::Extensions> > BufferedExtensions;
static BufferedExtensions s_extensions;

StencilTwoSided::Extensions* StencilTwoSided::getExtensions(unsigned int contextID,bool createIfNotInitalized)
{
    if (!s_extensions[contextID] && createIfNotInitalized) s_extensions[contextID] = new Extensions(contextID);
    return s_extensions[contextID].get();
}

void StencilTwoSided::setExtensions(unsigned int contextID,Extensions* extensions)
{
    s_extensions[contextID] = extensions;
}

StencilTwoSided::Extensions::Extensions(unsigned int contextID)
{
    setupGLExtensions(contextID);
}

StencilTwoSided::Extensions::Extensions(const Extensions& rhs):
    Referenced()
{
    _isStencilTwoSidedSupported = rhs._isStencilTwoSidedSupported;
    _glActiveStencilFace = rhs._glActiveStencilFace;
}


void StencilTwoSided::Extensions::lowestCommonDenominator(const Extensions& rhs)
{
    if (!rhs._isStencilTwoSidedSupported) _isStencilTwoSidedSupported = false;

    if (!rhs._glActiveStencilFace) _glActiveStencilFace = 0;

}

void StencilTwoSided::Extensions::setupGLExtensions(unsigned int contextID)
{
    _isStencilTwoSidedSupported = isGLExtensionSupported(contextID,"GL_EXT_stencil_two_side");

    setGLExtensionFuncPtr(_glActiveStencilFace, "glActiveStencilFace","glActiveStencilFaceEXT");
}

void StencilTwoSided::Extensions::glActiveStencilFace(GLenum face) const
{
    if (_glActiveStencilFace)
    {
        _glActiveStencilFace(face);
    }
    else
    {
        notify(WARN)<<"Error: glActiveStencilFace not supported by OpenGL driver"<<std::endl;
    }    
}

