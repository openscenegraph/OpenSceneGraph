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
    const GLExtensions* extensions = state.get<GLExtensions>();

    // use OpenGL 2.0 functions if available
    if (extensions->isOpenGL20Supported)
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
    if (extensions->isStencilTwoSidedSupported)
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
    if (extensions->isSeparateStencilSupported)
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
