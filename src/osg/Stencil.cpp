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

static Stencil::Operation validateOperation(State& state,
        const Stencil::Extensions* extensions, Stencil::Operation op)
{
    // only wrap requires validation
    if (op != Stencil::INCR_WRAP && op != Stencil::DECR_WRAP)
        return op;

    // get extension object
    if (!extensions)
    {
        const unsigned int contextID = state.getContextID();
        extensions = Stencil::getExtensions(contextID, true);
    }

    // wrap support
    if (extensions->isStencilWrapSupported())
        return op;
    else
        return op==Stencil::INCR_WRAP ? Stencil::INCR : Stencil::DECR;
}

void Stencil::apply(State& state) const
{
    const Extensions* extensions = NULL;
    Operation sf = validateOperation(state, extensions, _sfail);
    Operation zf = validateOperation(state, extensions, _zfail);
    Operation zp = validateOperation(state, extensions, _zpass);

    glStencilFunc((GLenum)_func,_funcRef,_funcMask);
    glStencilOp((GLenum)sf,(GLenum)zf,(GLenum)zp);
    glStencilMask(_writeMask);
}


typedef buffered_value< ref_ptr<Stencil::Extensions> > BufferedExtensions;
static BufferedExtensions s_extensions;

Stencil::Extensions* Stencil::getExtensions(unsigned int contextID, bool createIfNotInitalized)
{
    if (!s_extensions[contextID] && createIfNotInitalized)
        s_extensions[contextID] = new Extensions(contextID);
    return s_extensions[contextID].get();
}

void Stencil::setExtensions(unsigned int contextID, Extensions* extensions)
{
    s_extensions[contextID] = extensions;
}

Stencil::Extensions::Extensions(unsigned int contextID)
{
    setupGLExtensions(contextID);
}

Stencil::Extensions::Extensions(const Extensions& rhs) :
    Referenced()
{
    _isStencilWrapSupported = rhs._isStencilWrapSupported;
}


void Stencil::Extensions::lowestCommonDenominator(const Extensions& rhs)
{
    if (!rhs._isStencilWrapSupported)
        _isStencilWrapSupported = false;
}

void Stencil::Extensions::setupGLExtensions(unsigned int contextID)
{
    _isStencilWrapSupported = isGLExtensionOrVersionSupported(contextID, "GL_EXT_stencil_wrap", 1.4f);

    OSG_INFO << "Stencil wrap: " << (_isStencilWrapSupported ? "supported" : "not supported") << std::endl;
}
