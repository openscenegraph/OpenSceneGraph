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
#include <osg/Stencil>
#include <osg/GLExtensions>
#include <osg/State>
#include <osg/Notify>

using namespace osg;

Stencil::Stencil()
{
    // set up same defaults as glStencilFunc.
    _func = ALWAYS;
    _funcRef = 0;
    _funcMask = ~0u;

    // set up same defaults as glStencilOp.
    _sfail = KEEP;
    _zfail = KEEP;
    _zpass = KEEP;

    // set up same defaults as glStencilMask.
    _writeMask = ~0u;
}

Stencil::~Stencil()
{
}

static Stencil::Operation validateOperation(const GLExtensions* extensions, Stencil::Operation op)
{
    // only wrap requires validation
    if (op != Stencil::INCR_WRAP && op != Stencil::DECR_WRAP)
        return op;

    // wrap support
    if (extensions->isStencilWrapSupported)
        return op;
    else
        return op==Stencil::INCR_WRAP ? Stencil::INCR : Stencil::DECR;
}

void Stencil::apply(State& state) const
{
    const GLExtensions* extensions = state.get<GLExtensions>();
    Operation sf = validateOperation(extensions, _sfail);
    Operation zf = validateOperation(extensions, _zfail);
    Operation zp = validateOperation(extensions, _zpass);

    glStencilFunc((GLenum)_func,_funcRef,_funcMask);
    glStencilOp((GLenum)sf,(GLenum)zf,(GLenum)zp);
    glStencilMask(_writeMask);
}
