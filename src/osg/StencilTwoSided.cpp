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
    // used by the COMPARE_StateAttribute_Parameter macros below.
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

    return 0; // passed all the above comparison macros, must be equal.
}

void StencilTwoSided::apply(State& state) const
{
    // get "per-context" extensions
    const unsigned int contextID = state.getContextID();
    const Extensions* extensions = getExtensions(contextID,true);

    // use OpenGL 2.0 functions if available
    if (extensions->isOpenGL20Supported())
    {
        // front face
        extensions->glStencilOpSeparate(GL_FRONT, (GLenum)_sfail[FRONT],(GLenum)_zfail[FRONT],(GLenum)_zpass[FRONT]);
        extensions->glStencilMaskSeparate(GL_FRONT, _writeMask[FRONT]);
        extensions->glStencilFuncSeparate(GL_FRONT, (GLenum)_func[FRONT],_funcRef[FRONT],_funcMask[FRONT]);

        // back face
        extensions->glStencilOpSeparate(GL_BACK, (GLenum)_sfail[BACK],(GLenum)_zfail[BACK],(GLenum)_zpass[BACK]);
        extensions->glStencilMaskSeparate(GL_BACK, _writeMask[BACK]);
        extensions->glStencilFuncSeparate(GL_BACK, (GLenum)_func[BACK],_funcRef[BACK],_funcMask[BACK]);

        return;
    }

    // try to use GL_EXT_stencil_two_side extension
    if (extensions->isStencilTwoSidedSupported())
    {
        // enable two sided stenciling
        glEnable(GL_STENCIL_TEST_TWO_SIDE);

        // back face
        extensions->glActiveStencilFace(GL_BACK);
        glStencilOp((GLenum)_sfail[BACK],(GLenum)_zfail[BACK],(GLenum)_zpass[BACK]);
        glStencilMask(_writeMask[BACK]);
        glStencilFunc((GLenum)_func[BACK],_funcRef[BACK],_funcMask[BACK]);

        // front face
        extensions->glActiveStencilFace(GL_FRONT);
        glStencilOp((GLenum)_sfail[FRONT],(GLenum)_zfail[FRONT],(GLenum)_zpass[FRONT]);
        glStencilMask(_writeMask[FRONT]);
        glStencilFunc((GLenum)_func[FRONT],_funcRef[FRONT],_funcMask[FRONT]);

        return;
    }

    // try to use GL_ATI_separate_stencil extension
    if (extensions->isSeparateStencilSupported())
    {
        if( _writeMask[FRONT] != _writeMask[BACK] ||
            _funcRef[FRONT] != _funcRef[BACK] ||
            _funcMask[FRONT] != _funcMask[BACK] )
        {
            OSG_WARN << "StencilTwoSided uses GL_ATI_separate_stencil and there are different\n"
                        "   write mask, functionRef or functionMask values for the front and back\n"
                        "   faces. This is not supported by the extension. Using front values only." << std::endl;
        }

        glStencilMask(_writeMask[FRONT]);

        // front face
        extensions->glStencilOpSeparate(GL_FRONT, (GLenum)_sfail[FRONT], (GLenum)_zfail[FRONT], (GLenum)_zpass[FRONT]);
        extensions->glStencilFuncSeparateATI((GLenum)_func[FRONT], (GLenum)_func[BACK], _funcRef[FRONT], _funcMask[FRONT]);

        // back face
        extensions->glStencilOpSeparate(GL_BACK, (GLenum)_sfail[BACK], (GLenum)_zfail[BACK], (GLenum)_zpass[BACK]);
        extensions->glStencilFuncSeparateATI((GLenum)_func[FRONT], (GLenum)_func[BACK], _funcRef[FRONT], _funcMask[FRONT]);

        return;
    }

    OSG_WARN << "StencilTwoSided failed as the required graphics capabilities were\n"
                "   not found (contextID " << contextID << "). OpenGL 2.0 or one of extensions\n"
                "   GL_EXT_stencil_two_side or GL_ATI_separate_stencil is required." << std::endl;
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
    _isOpenGL20Supported = rhs._isOpenGL20Supported;
    _isSeparateStencilSupported = rhs._isSeparateStencilSupported;
    _glActiveStencilFace = rhs._glActiveStencilFace;
    _glStencilOpSeparate = rhs._glStencilOpSeparate;
    _glStencilMaskSeparate = rhs._glStencilMaskSeparate;
    _glStencilFuncSeparate = rhs._glStencilFuncSeparate;
    _glStencilFuncSeparateATI = rhs._glStencilFuncSeparateATI;
}


void StencilTwoSided::Extensions::lowestCommonDenominator(const Extensions& rhs)
{
    if (!rhs._isStencilTwoSidedSupported) _isStencilTwoSidedSupported = false;
    if (!rhs._isOpenGL20Supported) _isOpenGL20Supported = false;
    if (!rhs._isSeparateStencilSupported) _isSeparateStencilSupported = false;

    if (!rhs._glActiveStencilFace) _glActiveStencilFace = NULL;
    if (!rhs._glStencilOpSeparate) _glStencilOpSeparate = NULL;
    if (!rhs._glStencilMaskSeparate) _glStencilMaskSeparate = NULL;
    if (!rhs._glStencilFuncSeparate) _glStencilFuncSeparate = NULL;
    if (!rhs._glStencilFuncSeparateATI) _glStencilFuncSeparateATI = NULL;
}

void StencilTwoSided::Extensions::setupGLExtensions(unsigned int contextID)
{
    // extension support
    _isStencilTwoSidedSupported = isGLExtensionSupported(contextID, "GL_EXT_stencil_two_side");
    _isOpenGL20Supported = getGLVersionNumber() >= 2.0;
    _isSeparateStencilSupported = isGLExtensionSupported(contextID, "GL_ATI_separate_stencil");

    // function pointers
    setGLExtensionFuncPtr(_glActiveStencilFace, "glActiveStencilFaceEXT");
    setGLExtensionFuncPtr(_glStencilOpSeparate, "glStencilOpSeparate", "glStencilOpSeparateATI");
    setGLExtensionFuncPtr(_glStencilMaskSeparate, "glStencilMaskSeparate");
    setGLExtensionFuncPtr(_glStencilFuncSeparate, "glStencilFuncSeparate");
    setGLExtensionFuncPtr(_glStencilFuncSeparateATI, "glStencilFuncSeparateATI");

    // protect against buggy drivers (maybe not necessary)
    if (!_glActiveStencilFace) _isStencilTwoSidedSupported = false;
    if (!_glStencilOpSeparate) { _isOpenGL20Supported = false; _isSeparateStencilSupported = false; }
    if (!_glStencilMaskSeparate) _isOpenGL20Supported = false;
    if (!_glStencilFuncSeparate) _isOpenGL20Supported = false;
    if (!_glStencilFuncSeparateATI) _isSeparateStencilSupported = false;

    // notify
    if( _isOpenGL20Supported )
    {
       OSG_INFO << "StencilTwoSided is going to use OpenGL 2.0 API (contextID " << contextID << ")." << std::endl;
    }
    else if( _isStencilTwoSidedSupported )
    {
       OSG_INFO << "StencilTwoSided is going to use GL_EXT_stencil_two_side extension (contextID " << contextID << ")." << std::endl;
    }
    else if( _isSeparateStencilSupported )
    {
       OSG_INFO << "StencilTwoSided is going to use GL_ATI_separate_stencil extension (contextID " << contextID << ")." << std::endl;
    }
    else
    {
       OSG_INFO << "StencilTwoSided did not found required graphics capabilities\n"
                   "   (contextID " << contextID << "). OpenGL 2.0 or one of extensions\n"
                   "   GL_EXT_stencil_two_side or GL_ATI_separate_stencil is required." << std::endl;
    }
}

void StencilTwoSided::Extensions::glActiveStencilFace(GLenum face) const
{
    if (_isStencilTwoSidedSupported)
    {
        _glActiveStencilFace(face);
    }
}

void StencilTwoSided::Extensions::glStencilOpSeparate(GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass) const
{
    if (_isOpenGL20Supported || _isSeparateStencilSupported)
    {
        _glStencilOpSeparate(face, sfail, dpfail, dppass);
    }
}

void StencilTwoSided::Extensions::glStencilMaskSeparate(GLenum face, GLuint mask) const
{
    if (_isOpenGL20Supported)
    {
        _glStencilMaskSeparate(face, mask);
    }
}

void StencilTwoSided::Extensions::glStencilFuncSeparate(GLenum face, GLenum func, GLint ref, GLuint mask) const
{
    if (_isOpenGL20Supported)
    {
        _glStencilFuncSeparate(face, func, ref, mask);
    }
}

void StencilTwoSided::Extensions::glStencilFuncSeparateATI(GLenum frontfunc, GLenum backfunc, GLint ref, GLuint mask) const
{
    if (_isSeparateStencilSupported)
    {
        _glStencilFuncSeparateATI(frontfunc, backfunc, ref, mask);
    }
}
