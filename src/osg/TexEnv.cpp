#include "osg/TexEnv"

using namespace osg;

TexEnv::TexEnv()
{
    _mode = MODULATE;
}


TexEnv::~TexEnv()
{
}

void TexEnv::setMode( const Mode mode )
{
    _mode = (mode == DECAL ||
        mode == MODULATE ||
        mode == BLEND ||
        mode == REPLACE ) ?
        mode : MODULATE;
}


void TexEnv::apply(State&) const
{
    glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, _mode);
}
