#include <osg/Stencil>

using namespace osg;

Stencil::Stencil()
{
    // set up same defaults as glStencilFunc.
    _func = ALWAYS;
    _funcRef = 0;
    _funcMask = ~0;
        
    // set up same defaults as glStencilOp.
    _sfail = KEEP;
    _zfail = KEEP;
    _zpass = KEEP;

    // set up same defaults as glStencilMask.
    _writeMask = ~0;
}


Stencil::~Stencil()
{
}

void Stencil::apply(State&) const
{
    glStencilFunc((GLenum)_func,_funcRef,_funcMask);
    glStencilOp((GLenum)_sfail,(GLenum)_zfail,(GLenum)_zpass);
    glStencilMask(_writeMask);
}

