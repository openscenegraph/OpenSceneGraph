#include <osg/GLExtensions>
#include <osg/TexEnvCombine>

using namespace osg;

TexEnvCombine::TexEnvCombine():
            _needsTexEnvCrossbar(false),
            _combine_RGB(GL_MODULATE),
            _combine_Alpha(GL_MODULATE),
            _source0_RGB(GL_TEXTURE),
            _source1_RGB(GL_PREVIOUS_ARB),
            _source2_RGB(GL_CONSTANT_ARB),
            _source0_Alpha(GL_TEXTURE),
            _source1_Alpha(GL_PREVIOUS_ARB),
            _source2_Alpha(GL_CONSTANT_ARB),
            _operand0_RGB(GL_SRC_COLOR),
            _operand1_RGB(GL_SRC_COLOR),
            _operand2_RGB(GL_SRC_ALPHA),
            _operand0_Alpha(GL_SRC_ALPHA),
            _operand1_Alpha(GL_SRC_ALPHA),
            _operand2_Alpha(GL_SRC_ALPHA),
            _scale_RGB(1.0),
            _scale_Alpha(1.0),
            _constantColor(0.0f,0.0f,0.0f,0.0f)
{
}

TexEnvCombine::~TexEnvCombine()
{
}

void TexEnvCombine::apply(State&) const
{
    static bool s_isTexEnvCombineSupported =
        isGLExtensionSupported("GL_ARB_texture_env_combine");

    static bool s_isTexEnvCrossbarSupported =
        isGLExtensionSupported("GL_ARB_texture_env_crossbar");

    static bool s_isTexEnvDot3Supported = 
        isGLExtensionSupported("GL_ARB_texture_env_dot3");

    bool needsTexEnvDot3 = (_combine_RGB==DOT3_RGB) ||
                           (_combine_RGB==DOT3_RGBA);

    bool supported = s_isTexEnvCombineSupported;
    if (_needsTexEnvCrossbar && !s_isTexEnvCrossbarSupported) supported = false;
    if (needsTexEnvDot3 && !s_isTexEnvDot3Supported) supported = false;

    if (supported)
    {
        glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);

        glTexEnvi( GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, _combine_RGB);
        
        if (_combine_RGB!=DOT3_RGBA)
            glTexEnvi( GL_TEXTURE_ENV, GL_COMBINE_ALPHA_ARB, _combine_Alpha);

        glTexEnvi( GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB,_source0_RGB );
        glTexEnvi( GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, _source1_RGB);
        glTexEnvi( GL_TEXTURE_ENV, GL_SOURCE2_RGB_ARB,_source2_RGB );

        glTexEnvi( GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_ARB, _source0_Alpha);
        glTexEnvi( GL_TEXTURE_ENV, GL_SOURCE1_ALPHA_ARB, _source1_Alpha);
        glTexEnvi( GL_TEXTURE_ENV, GL_SOURCE2_ALPHA_ARB, _source2_Alpha);

        glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND0_RGB_ARB, _operand0_RGB);
        glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND1_RGB_ARB, _operand1_RGB);
        glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND2_RGB_ARB, _operand2_RGB);

        glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_ARB, _operand0_Alpha);
        glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND1_ALPHA_ARB, _operand1_Alpha);
        glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND2_ALPHA_ARB, _operand2_Alpha);

        glTexEnvf( GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, _scale_RGB);
        glTexEnvf( GL_TEXTURE_ENV, GL_ALPHA_SCALE, _scale_Alpha);

        glTexEnvfv( GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, _constantColor.ptr());
    }
    else
    {
        // what is the best fallback when the tex env combine is not supported??
        // we will resort the settung the OpenGL default of GL_MODULATE.
        glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    }
}
