#include <osg/GLExtensions>
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
    if (_mode==ADD)
    {
        static bool isTexEnvAddSupported = isGLExtensionSupported("GL_ARB_texture_env_add");
        if (isTexEnvAddSupported)
            glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, ADD);
        else // fallback on OpenGL default.
            glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, MODULATE);
    }
    else
    {
        glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, _mode);
        if (_mode==TexEnv::BLEND)
        {
            glTexEnvfv( GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, _color.ptr());
        }
    }
}
