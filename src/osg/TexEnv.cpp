#include <osg/TexEnv>

using namespace osg;

TexEnv::TexEnv()
{
    _mode = MODULATE;
    _color.set(0.0f,0.0f,0.0f,0.0f);
}


TexEnv::~TexEnv()
{
}

void TexEnv::apply(State&) const
{
    glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, _mode);
    if (_mode==TexEnv::BLEND)
    {
        glTexEnvfv( GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, _color.ptr());
    }
}
