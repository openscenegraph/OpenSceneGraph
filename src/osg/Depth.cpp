#include <osg/Depth>

using namespace osg;

Depth::Depth()
{
    _func = LESS;
    _depthWriteMask = true;

    _zNear = 0.0;
    _zFar = 1.0;
}


Depth::~Depth()
{
}

void Depth::apply(State&) const
{
    glDepthFunc((GLenum)_func);
    glDepthMask((GLboolean)_depthWriteMask);
    glDepthRange(_zNear,_zFar);
}

