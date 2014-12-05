/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield
 * Copyright (C) 2003-2005 3Dlabs Inc. Ltd.
 * Copyright (C) 2004-2005 Nathan Cournia
 * Copyright (C) 2008 Zebra Imaging
 * Copyright (C) 2012 David Callu
 *
 * This application is open source and may be redistributed and/or modified
 * freely and without restriction, both in commercial and non commercial
 * applications, as long as this copyright notice is maintained.
 *
 * This application is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
*/

/* file:        src/osg/GL2Extensions.cpp
 * author:      Mike Weiblen 2008-01-19
*/

#include <osg/Notify>
#include <osg/buffered_value>
#include <osg/ref_ptr>
#include <osg/GL2Extensions>
#include <osg/GLExtensions>
#include <osg/Math>

using namespace osg;


///////////////////////////////////////////////////////////////////////////
// Static array of percontext osg::GL2Extensions instances

typedef osg::buffered_object< osg::ref_ptr<GL2Extensions> > BufferedExtensions;
static BufferedExtensions s_extensions;

GL2Extensions* GL2Extensions::Get(unsigned int contextID, bool createIfNotInitalized)
{
    if (!s_extensions[contextID] && createIfNotInitalized)
            s_extensions[contextID] = new GL2Extensions(contextID);

    return s_extensions[contextID].get();
}

void GL2Extensions::Set(unsigned int contextID, GL2Extensions* extensions)
{
    s_extensions[contextID] = extensions;
}

///////////////////////////////////////////////////////////////////////////
// Extension function pointers for OpenGL v2.x

GL2Extensions::GL2Extensions(unsigned int contextID)
{
    const char* version = (const char*) glGetString( GL_VERSION );
    if (!version)
    {
        OSG_NOTIFY(osg::FATAL)<<"Error: OpenGL version test failed, requires valid graphics context."<<std::endl;
        return;
    }

    glVersion = findAsciiToFloat( version );
    glslLanguageVersion = 0.0f;

    bool shadersBuiltIn = OSG_GLES2_FEATURES || OSG_GL3_FEATURES;

    isShaderObjectsSupported = shadersBuiltIn || osg::isGLExtensionSupported(contextID,"GL_ARB_shader_objects");
    isVertexShaderSupported = shadersBuiltIn || osg::isGLExtensionSupported(contextID,"GL_ARB_vertex_shader");
    isFragmentShaderSupported = shadersBuiltIn || osg::isGLExtensionSupported(contextID,"GL_ARB_fragment_shader");
    isLanguage100Supported = shadersBuiltIn || osg::isGLExtensionSupported(contextID,"GL_ARB_shading_language_100");
    isGeometryShader4Supported = osg::isGLExtensionSupported(contextID,"GL_EXT_geometry_shader4");
    isGpuShader4Supported = osg::isGLExtensionSupported(contextID,"GL_EXT_gpu_shader4");
    areTessellationShadersSupported = osg::isGLExtensionSupported(contextID, "GL_ARB_tessellation_shader");
    isUniformBufferObjectSupported = osg::isGLExtensionSupported(contextID,"GL_ARB_uniform_buffer_object");
    isGetProgramBinarySupported = osg::isGLExtensionSupported(contextID,"GL_ARB_get_program_binary");
    isGpuShaderFp64Supported = osg::isGLExtensionSupported(contextID,"GL_ARB_gpu_shader_fp64");
    isShaderAtomicCountersSupported = osg::isGLExtensionSupported(contextID,"GL_ARB_shader_atomic_counters");

    isGlslSupported = ( glVersion >= 2.0f ) ||
                      ( isShaderObjectsSupported &&
                        isVertexShaderSupported &&
                        isFragmentShaderSupported &&
                        isLanguage100Supported );

    if( isGlslSupported )
    {
        // If glGetString raises an error, assume initial release "1.00"
        while(glGetError() != GL_NO_ERROR) {}        // reset error flag

        const char* langVerStr = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
        if( (glGetError() == GL_NO_ERROR) && langVerStr )
        {
            glslLanguageVersion = (findAsciiToFloat(langVerStr));
        }
        else
            glslLanguageVersion = 1.0f;
    }

    OSG_INFO
            << "glVersion=" << glVersion << ", "
            << "isGlslSupported=" << (isGlslSupported ? "YES" : "NO") << ", "
            << "glslLanguageVersion=" << glslLanguageVersion
            << std::endl;


    setGLExtensionFuncPtr(glBlendEquationSeparate, "glBlendEquationSeparate");
    setGLExtensionFuncPtr(glDrawBuffers, "glDrawBuffers", "glDrawBuffersARB");
    setGLExtensionFuncPtr(glStencilOpSeparate, "glStencilOpSeparate");
    setGLExtensionFuncPtr(glStencilFuncSeparate, "glStencilFuncSeparate");
    setGLExtensionFuncPtr(glStencilMaskSeparate, "glStencilMaskSeparate");
    setGLExtensionFuncPtr(glAttachShader, "glAttachShader", "glAttachObjectARB");
    setGLExtensionFuncPtr(glBindAttribLocation, "glBindAttribLocation", "glBindAttribLocationARB");
    setGLExtensionFuncPtr(glCompileShader, "glCompileShader", "glCompileShaderARB");
    setGLExtensionFuncPtr(glCreateProgram, "glCreateProgram", "glCreateProgramObjectARB");
    setGLExtensionFuncPtr(glCreateShader, "glCreateShader", "glCreateShaderObjectARB");
    setGLExtensionFuncPtr(glDeleteProgram, "glDeleteProgram");
    setGLExtensionFuncPtr(glDeleteShader, "glDeleteShader");
    setGLExtensionFuncPtr(glDetachShader, "glDetachShader", "glDetachObjectARB");
    setGLExtensionFuncPtr(glDisableVertexAttribArray, "glDisableVertexAttribArray");
    setGLExtensionFuncPtr(glEnableVertexAttribArray, "glEnableVertexAttribArray");
    setGLExtensionFuncPtr(glGetActiveAttrib, "glGetActiveAttrib", "glGetActiveAttribARB");
    setGLExtensionFuncPtr(glGetActiveUniform, "glGetActiveUniform", "glGetActiveUniformARB");
    setGLExtensionFuncPtr(glGetAttachedShaders, "glGetAttachedShaders", "glGetAttachedObjectsARB");
    setGLExtensionFuncPtr(glGetAttribLocation, "glGetAttribLocation", "glGetAttribLocationARB");
    setGLExtensionFuncPtr(glGetProgramiv, "glGetProgramiv");
    setGLExtensionFuncPtr(glGetProgramInfoLog, "glGetProgramInfoLog");
    setGLExtensionFuncPtr(glGetShaderiv, "glGetShaderiv");
    setGLExtensionFuncPtr(glGetShaderInfoLog, "glGetShaderInfoLog");
    setGLExtensionFuncPtr(glGetShaderSource, "glGetShaderSource", "glGetShaderSourceARB");
    setGLExtensionFuncPtr(glGetUniformLocation, "glGetUniformLocation", "glGetUniformLocationARB");
    setGLExtensionFuncPtr(glGetUniformfv, "glGetUniformfv", "glGetUniformfvARB");
    setGLExtensionFuncPtr(glGetUniformiv, "glGetUniformiv", "glGetUniformivARB");
    setGLExtensionFuncPtr(glGetVertexAttribdv, "glGetVertexAttribdv");
    setGLExtensionFuncPtr(glGetVertexAttribfv, "glGetVertexAttribfv");
    setGLExtensionFuncPtr(glGetVertexAttribiv, "glGetVertexAttribiv");
    setGLExtensionFuncPtr(glGetVertexAttribPointerv, "glGetVertexAttribPointerv");
    setGLExtensionFuncPtr(glIsProgram, "glIsProgram");
    setGLExtensionFuncPtr(glIsShader, "glIsShader");
    setGLExtensionFuncPtr(glLinkProgram, "glLinkProgram", "glLinkProgramARB");
    setGLExtensionFuncPtr(glShaderSource, "glShaderSource", "glShaderSourceARB");
    setGLExtensionFuncPtr(glUseProgram, "glUseProgram", "glUseProgramObjectARB");
    setGLExtensionFuncPtr(glUniform1f, "glUniform1f", "glUniform1fARB");
    setGLExtensionFuncPtr(glUniform2f, "glUniform2f", "glUniform2fARB");
    setGLExtensionFuncPtr(glUniform3f, "glUniform3f", "glUniform3fARB");
    setGLExtensionFuncPtr(glUniform4f, "glUniform4f", "glUniform4fARB");
    setGLExtensionFuncPtr(glUniform1i, "glUniform1i", "glUniform1iARB");
    setGLExtensionFuncPtr(glUniform2i, "glUniform2i", "glUniform2iARB");
    setGLExtensionFuncPtr(glUniform3i, "glUniform3i", "glUniform3iARB");
    setGLExtensionFuncPtr(glUniform4i, "glUniform4i", "glUniform4iARB");
    setGLExtensionFuncPtr(glUniform1fv, "glUniform1fv", "glUniform1fvARB");
    setGLExtensionFuncPtr(glUniform2fv, "glUniform2fv", "glUniform2fvARB");
    setGLExtensionFuncPtr(glUniform3fv, "glUniform3fv", "glUniform3fvARB");
    setGLExtensionFuncPtr(glUniform4fv, "glUniform4fv", "glUniform4fvARB");
    setGLExtensionFuncPtr(glUniform1iv, "glUniform1iv", "glUniform1ivARB");
    setGLExtensionFuncPtr(glUniform2iv, "glUniform2iv", "glUniform2ivARB");
    setGLExtensionFuncPtr(glUniform3iv, "glUniform3iv", "glUniform3ivARB");
    setGLExtensionFuncPtr(glUniform4iv, "glUniform4iv", "glUniform4ivARB");
    setGLExtensionFuncPtr(glUniformMatrix2fv, "glUniformMatrix2fv", "glUniformMatrix2fvARB");
    setGLExtensionFuncPtr(glUniformMatrix3fv, "glUniformMatrix3fv", "glUniformMatrix3fvARB");
    setGLExtensionFuncPtr(glUniformMatrix4fv, "glUniformMatrix4fv", "glUniformMatrix4fvARB");
    setGLExtensionFuncPtr(glValidateProgram, "glValidateProgram", "glValidateProgramARB");
    setGLExtensionFuncPtr(glVertexAttrib1d, "glVertexAttrib1d");
    setGLExtensionFuncPtr(glVertexAttrib1dv, "glVertexAttrib1dv");
    setGLExtensionFuncPtr(glVertexAttrib1f, "glVertexAttrib1f");
    setGLExtensionFuncPtr(glVertexAttrib1fv, "glVertexAttrib1fv");
    setGLExtensionFuncPtr(glVertexAttrib1s, "glVertexAttrib1s");
    setGLExtensionFuncPtr(glVertexAttrib1sv, "glVertexAttrib1sv");
    setGLExtensionFuncPtr(glVertexAttrib2d, "glVertexAttrib2d");
    setGLExtensionFuncPtr(glVertexAttrib2dv, "glVertexAttrib2dv");
    setGLExtensionFuncPtr(glVertexAttrib2f, "glVertexAttrib2f");
    setGLExtensionFuncPtr(glVertexAttrib2fv, "glVertexAttrib2fv");
    setGLExtensionFuncPtr(glVertexAttrib2s, "glVertexAttrib2s");
    setGLExtensionFuncPtr(glVertexAttrib2sv, "glVertexAttrib2sv");
    setGLExtensionFuncPtr(glVertexAttrib3d, "glVertexAttrib3d");
    setGLExtensionFuncPtr(glVertexAttrib3dv, "glVertexAttrib3dv");
    setGLExtensionFuncPtr(glVertexAttrib3f, "glVertexAttrib3f");
    setGLExtensionFuncPtr(glVertexAttrib3fv, "glVertexAttrib3fv");
    setGLExtensionFuncPtr(glVertexAttrib3s, "glVertexAttrib3s");
    setGLExtensionFuncPtr(glVertexAttrib3sv, "glVertexAttrib3sv");
    setGLExtensionFuncPtr(glVertexAttrib4Nbv, "glVertexAttrib4Nbv");
    setGLExtensionFuncPtr(glVertexAttrib4Niv, "glVertexAttrib4Niv");
    setGLExtensionFuncPtr(glVertexAttrib4Nsv, "glVertexAttrib4Nsv");
    setGLExtensionFuncPtr(glVertexAttrib4Nub, "glVertexAttrib4Nub");
    setGLExtensionFuncPtr(glVertexAttrib4Nubv, "glVertexAttrib4Nubv");
    setGLExtensionFuncPtr(glVertexAttrib4Nuiv, "glVertexAttrib4Nuiv");
    setGLExtensionFuncPtr(glVertexAttrib4Nusv, "glVertexAttrib4Nusv");
    setGLExtensionFuncPtr(glVertexAttrib4bv, "glVertexAttrib4bv");
    setGLExtensionFuncPtr(glVertexAttrib4d, "glVertexAttrib4d");
    setGLExtensionFuncPtr(glVertexAttrib4dv, "glVertexAttrib4dv");
    setGLExtensionFuncPtr(glVertexAttrib4f, "glVertexAttrib4f");
    setGLExtensionFuncPtr(glVertexAttrib4fv, "glVertexAttrib4fv");
    setGLExtensionFuncPtr(glVertexAttrib4iv, "glVertexAttrib4iv");
    setGLExtensionFuncPtr(glVertexAttrib4s, "glVertexAttrib4s");
    setGLExtensionFuncPtr(glVertexAttrib4sv, "glVertexAttrib4sv");
    setGLExtensionFuncPtr(glVertexAttrib4ubv, "glVertexAttrib4ubv");
    setGLExtensionFuncPtr(glVertexAttrib4uiv, "glVertexAttrib4uiv");
    setGLExtensionFuncPtr(glVertexAttrib4usv, "glVertexAttrib4usv");
    setGLExtensionFuncPtr(glVertexAttribPointer, "glVertexAttribPointer");
    setGLExtensionFuncPtr(glVertexAttribDivisor, "glVertexAttribDivisor");

    // v1.5-only ARB entry points, in case they're needed for fallback
    setGLExtensionFuncPtr(glGetInfoLogARB, "glGetInfoLogARB");
    setGLExtensionFuncPtr(glGetObjectParameterivARB, "glGetObjectParameterivARB");
    setGLExtensionFuncPtr(glDeleteObjectARB, "glDeleteObjectARB");
    setGLExtensionFuncPtr(glGetHandleARB, "glGetHandleARB");

    // GL 2.1
    setGLExtensionFuncPtr(glUniformMatrix2x3fv,  "glUniformMatrix2x3fv" );
    setGLExtensionFuncPtr(glUniformMatrix3x2fv,  "glUniformMatrix3x2fv" );
    setGLExtensionFuncPtr(glUniformMatrix2x4fv,  "glUniformMatrix2x4fv" );
    setGLExtensionFuncPtr(glUniformMatrix4x2fv,  "glUniformMatrix4x2fv" );
    setGLExtensionFuncPtr(glUniformMatrix3x4fv,  "glUniformMatrix3x4fv" );
    setGLExtensionFuncPtr(glUniformMatrix4x3fv,  "glUniformMatrix4x3fv" );

    // EXT_geometry_shader4
    setGLExtensionFuncPtr(glProgramParameteri,  "glProgramParameteri", "glProgramParameteriEXT" );
    setGLExtensionFuncPtr(glFramebufferTexture,  "glFramebufferTexture", "glFramebufferTextureEXT" );
    setGLExtensionFuncPtr(glFramebufferTextureLayer,  "glFramebufferTextureLayer", "glFramebufferTextureLayerEXT" );
    setGLExtensionFuncPtr(glFramebufferTextureFace,  "glFramebufferTextureFace", "glFramebufferTextureFaceEXT" );

    // ARB_tesselation_shader
    setGLExtensionFuncPtr(glPatchParameteri, "glPatchParameteri" );
    setGLExtensionFuncPtr(glPatchParameterfv, "glPatchParameterfv");

    // EXT_gpu_shader4
    setGLExtensionFuncPtr(glGetUniformuiv,  "glGetUniformuiv", "glGetUniformuivEXT" );
    setGLExtensionFuncPtr(glBindFragDataLocation,  "glBindFragDataLocation", "glBindFragDataLocationEXT" );
    setGLExtensionFuncPtr(glGetFragDataLocation,  "glGetFragDataLocation", "glGetFragDataLocationEXT" );
    setGLExtensionFuncPtr(glUniform1ui,  "glUniform1ui", "glUniform1uiEXT" );
    setGLExtensionFuncPtr(glUniform2ui,  "glUniform2ui", "glUniform2uiEXT" );
    setGLExtensionFuncPtr(glUniform3ui,  "glUniform3ui", "glUniform3uiEXT" );
    setGLExtensionFuncPtr(glUniform4ui,  "glUniform4ui", "glUniform4uiEXT" );
    setGLExtensionFuncPtr(glUniform1uiv,  "glUniform1uiv", "glUniform1uivEXT" );
    setGLExtensionFuncPtr(glUniform2uiv,  "glUniform2uiv", "glUniform2uivEXT" );
    setGLExtensionFuncPtr(glUniform3uiv,  "glUniform3uiv", "glUniform3uivEXT" );
    setGLExtensionFuncPtr(glUniform4uiv,  "glUniform4uiv", "glUniform4uivEXT" );
    // ARB_uniform_buffer_object
    setGLExtensionFuncPtr(glGetUniformIndices, "glGetUniformIndices");
    setGLExtensionFuncPtr(glGetActiveUniformsiv, "glGetActiveUniformsiv");
    setGLExtensionFuncPtr(glGetActiveUniformName, "glGetActiveUniformName");
    setGLExtensionFuncPtr(glGetUniformBlockIndex, "glGetUniformBlockIndex");
    setGLExtensionFuncPtr(glGetActiveUniformBlockiv, "glGetActiveUniformBlockiv");
    setGLExtensionFuncPtr(glGetActiveUniformBlockName, "glGetActiveUniformBlockName");
    setGLExtensionFuncPtr(glUniformBlockBinding, "glUniformBlockBinding");

    // ARB_get_program_binary
    setGLExtensionFuncPtr(glGetProgramBinary, "glGetProgramBinary");
    setGLExtensionFuncPtr(glProgramBinary, "glProgramBinary");

    // ARB_gpu_shader_fp64
    setGLExtensionFuncPtr(glUniform1d, "glUniform1d" );
    setGLExtensionFuncPtr(glUniform2d, "glUniform2d" );
    setGLExtensionFuncPtr(glUniform3d, "glUniform3d" );
    setGLExtensionFuncPtr(glUniform4d, "glUniform4d" );
    setGLExtensionFuncPtr(glUniform1dv, "glUniform1dv" );
    setGLExtensionFuncPtr(glUniform2dv, "glUniform2dv" );
    setGLExtensionFuncPtr(glUniform3dv, "glUniform3dv" );
    setGLExtensionFuncPtr(glUniform4dv, "glUniform4dv" );
    setGLExtensionFuncPtr(glUniformMatrix2dv, "glUniformMatrix2dv" );
    setGLExtensionFuncPtr(glUniformMatrix3dv, "glUniformMatrix3dv" );
    setGLExtensionFuncPtr(glUniformMatrix4dv, "glUniformMatrix4dv" );
    setGLExtensionFuncPtr(glUniformMatrix2x3dv,  "glUniformMatrix2x3dv" );
    setGLExtensionFuncPtr(glUniformMatrix3x2dv,  "glUniformMatrix3x2dv" );
    setGLExtensionFuncPtr(glUniformMatrix2x4dv,  "glUniformMatrix2x4dv" );
    setGLExtensionFuncPtr(glUniformMatrix4x2dv,  "glUniformMatrix4x2dv" );
    setGLExtensionFuncPtr(glUniformMatrix3x4dv,  "glUniformMatrix3x4dv" );
    setGLExtensionFuncPtr(glUniformMatrix4x3dv,  "glUniformMatrix4x3dv" );

    // ARB_shader_atomic_counters
    setGLExtensionFuncPtr(glGetActiveAtomicCounterBufferiv,  "glGetActiveAtomicCounterBufferiv" );

    // ARB_compute_shader
    setGLExtensionFuncPtr(glDispatchCompute,  "glDispatchCompute" );

    setGLExtensionFuncPtr(glMemoryBarrier,  "glMemoryBarrier", "glMemoryBarrierEXT" );

    // BufferObject extensions
    setGLExtensionFuncPtr(glGenBuffers, "glGenBuffers","glGenBuffersARB");
    setGLExtensionFuncPtr(glBindBuffer, "glBindBuffer","glBindBufferARB");
    setGLExtensionFuncPtr(glBufferData, "glBufferData","glBufferDataARB");
    setGLExtensionFuncPtr(glBufferSubData, "glBufferSubData","glBufferSubDataARB");
    setGLExtensionFuncPtr(glDeleteBuffers, "glDeleteBuffers","glDeleteBuffersARB");
    setGLExtensionFuncPtr(glIsBuffer, "glIsBuffer","glIsBufferARB");
    setGLExtensionFuncPtr(glGetBufferSubData, "glGetBufferSubData","glGetBufferSubDataARB");
    setGLExtensionFuncPtr(glMapBuffer, "glMapBuffer","glMapBufferARB");
    setGLExtensionFuncPtr(glMapBufferRange,  "glMapBufferRange" );
    setGLExtensionFuncPtr(glUnmapBuffer, "glUnmapBuffer","glUnmapBufferARB");
    setGLExtensionFuncPtr(glGetBufferParameteriv, "glGetBufferParameteriv","glGetBufferParameterivARB");
    setGLExtensionFuncPtr(glGetBufferPointerv, "glGetBufferPointerv","glGetBufferPointervARB");
    setGLExtensionFuncPtr(glBindBufferRange, "glBindBufferRange");
    setGLExtensionFuncPtr(glBindBufferBase,  "glBindBufferBase", "glBindBufferBaseEXT", "glBindBufferBaseNV" );
    setGLExtensionFuncPtr(glTexBuffer, "glTexBuffer","glTexBufferARB" );

    isPBOSupported = OSG_GL3_FEATURES || osg::isGLExtensionSupported(contextID,"GL_ARB_pixel_buffer_object");
    isUniformBufferObjectSupported = osg::isGLExtensionSupported(contextID, "GL_ARB_uniform_buffer_object");
    isTBOSupported = osg::isGLExtensionSupported(contextID,"GL_ARB_texture_buffer_object");


    // BlendFunc extensions
    isBlendFuncSeparateSupported = OSG_GLES2_FEATURES || OSG_GL3_FEATURES ||
                                    osg::isGLExtensionSupported(contextID, "GL_EXT_blend_func_separate") ||
                                    strncmp((const char*)glGetString(GL_VERSION), "1.4", 3) >= 0;

    setGLExtensionFuncPtr(glBlendFuncSeparate, "glBlendFuncSeparate", "glBlendFuncSeparateEXT");

    setGLExtensionFuncPtr(glBlendFunci, "glBlendFunci", "glBlendFunciARB");
    setGLExtensionFuncPtr(glBlendFuncSeparatei, "glBlendFuncSeparatei", "glBlendFuncSeparateiARB");


    // Vertex Array extensions
    isVertexProgramSupported = isGLExtensionSupported(contextID,"GL_ARB_vertex_program");
    isSecondaryColorSupported = isGLExtensionSupported(contextID,"GL_EXT_secondary_color");
    isFogCoordSupported = isGLExtensionSupported(contextID,"GL_EXT_fog_coord");
    isMultiTexSupported = isGLExtensionSupported(contextID,"GL_ARB_multitexture");
    isOcclusionQuerySupported = osg::isGLExtensionSupported(contextID, "GL_NV_occlusion_query" );
    isARBOcclusionQuerySupported = OSG_GL3_FEATURES || osg::isGLExtensionSupported(contextID, "GL_ARB_occlusion_query" );

    isTimerQuerySupported = osg::isGLExtensionSupported(contextID, "GL_EXT_timer_query" );
    isARBTimerQuerySupported = osg::isGLExtensionSupported(contextID, "GL_ARB_timer_query");

    isRectangleSupported = OSG_GL3_FEATURES ||
                           isGLExtensionSupported(contextID,"GL_ARB_texture_rectangle") ||
                           isGLExtensionSupported(contextID,"GL_EXT_texture_rectangle") ||
                           isGLExtensionSupported(contextID,"GL_NV_texture_rectangle");


    setGLExtensionFuncPtr(glFogCoordfv, "glFogCoordfv","glFogCoordfvEXT");
    setGLExtensionFuncPtr(glSecondaryColor3ubv, "glSecondaryColor3ubv","glSecondaryColor3ubvEXT");
    setGLExtensionFuncPtr(glSecondaryColor3fv, "glSecondaryColor3fv","glSecondaryColor3fvEXT");
    setGLExtensionFuncPtr(glMultiTexCoord1f, "glMultiTexCoord1f","glMultiTexCoord1fARB");
    setGLExtensionFuncPtr(glMultiTexCoord1fv, "glMultiTexCoord1fv","glMultiTexCoord1fvARB");
    setGLExtensionFuncPtr(glMultiTexCoord2fv, "glMultiTexCoord2fv","glMultiTexCoord2fvARB");
    setGLExtensionFuncPtr(glMultiTexCoord3fv, "glMultiTexCoord3fv","glMultiTexCoord3fvARB");
    setGLExtensionFuncPtr(glMultiTexCoord4fv, "glMultiTexCoord4fv","glMultiTexCoord4fvARB");
    setGLExtensionFuncPtr(glMultiTexCoord1d, "glMultiTexCoord1d","glMultiTexCoorddfARB");
    setGLExtensionFuncPtr(glMultiTexCoord2dv, "glMultiTexCoord2dv","glMultiTexCoord2dvARB");
    setGLExtensionFuncPtr(glMultiTexCoord3dv, "glMultiTexCoord3dv","glMultiTexCoord3dvARB");
    setGLExtensionFuncPtr(glMultiTexCoord4dv, "glMultiTexCoord4dv","glMultiTexCoord4dvARB");

    setGLExtensionFuncPtr(glVertexAttrib1s, "glVertexAttrib1s","glVertexAttrib1sARB");
    setGLExtensionFuncPtr(glVertexAttrib1f, "glVertexAttrib1f","glVertexAttrib1fARB");
    setGLExtensionFuncPtr(glVertexAttrib1d, "glVertexAttrib1d","glVertexAttrib1dARB");
    setGLExtensionFuncPtr(glVertexAttrib1fv, "glVertexAttrib1fv","glVertexAttrib1fvARB");
    setGLExtensionFuncPtr(glVertexAttrib2fv, "glVertexAttrib2fv","glVertexAttrib2fvARB");
    setGLExtensionFuncPtr(glVertexAttrib3fv, "glVertexAttrib3fv","glVertexAttrib3fvARB");
    setGLExtensionFuncPtr(glVertexAttrib4fv, "glVertexAttrib4fv","glVertexAttrib4fvARB");
    setGLExtensionFuncPtr(glVertexAttrib2dv, "glVertexAttrib2dv","glVertexAttrib2dvARB");
    setGLExtensionFuncPtr(glVertexAttrib3dv, "glVertexAttrib3dv","glVertexAttrib3dvARB");
    setGLExtensionFuncPtr(glVertexAttrib4dv, "glVertexAttrib4dv","glVertexAttrib4dvARB");
    setGLExtensionFuncPtr(glVertexAttrib4ubv, "glVertexAttrib4ubv","glVertexAttrib4ubvARB");
    setGLExtensionFuncPtr(glVertexAttrib4Nubv, "glVertexAttrib4Nubv","glVertexAttrib4NubvARB");

    setGLExtensionFuncPtr(glGenBuffers, "glGenBuffers","glGenBuffersARB");
    setGLExtensionFuncPtr(glBindBuffer, "glBindBuffer","glBindBufferARB");
    setGLExtensionFuncPtr(glBufferData, "glBufferData","glBufferDataARB");
    setGLExtensionFuncPtr(glBufferSubData, "glBufferSubData","glBufferSubDataARB");
    setGLExtensionFuncPtr(glDeleteBuffers, "glDeleteBuffers","glDeleteBuffersARB");
    setGLExtensionFuncPtr(glIsBuffer, "glIsBuffer","glIsBufferARB");
    setGLExtensionFuncPtr(glGetBufferSubData, "glGetBufferSubData","glGetBufferSubDataARB");
    setGLExtensionFuncPtr(glMapBuffer, "glMapBuffer","glMapBufferARB");
    setGLExtensionFuncPtr(glUnmapBuffer, "glUnmapBuffer","glUnmapBufferARB");
    setGLExtensionFuncPtr(glGetBufferParameteriv, "glGetBufferParameteriv","glGetBufferParameterivARB");
    setGLExtensionFuncPtr(glGetBufferPointerv, "glGetBufferPointerv","glGetBufferPointervARB");

    setGLExtensionFuncPtr(glGenOcclusionQueries, "glGenOcclusionQueries","glGenOcclusionQueriesNV");
    setGLExtensionFuncPtr(glDeleteOcclusionQueries, "glDeleteOcclusionQueries","glDeleteOcclusionQueriesNV");
    setGLExtensionFuncPtr(glIsOcclusionQuery, "glIsOcclusionQuery","_glIsOcclusionQueryNV");
    setGLExtensionFuncPtr(glBeginOcclusionQuery, "glBeginOcclusionQuery","glBeginOcclusionQueryNV");
    setGLExtensionFuncPtr(glEndOcclusionQuery, "glEndOcclusionQuery","glEndOcclusionQueryNV");
    setGLExtensionFuncPtr(glGetOcclusionQueryiv, "glGetOcclusionQueryiv","glGetOcclusionQueryivNV");
    setGLExtensionFuncPtr(glGetOcclusionQueryuiv, "glGetOcclusionQueryuiv","glGetOcclusionQueryuivNV");

    setGLExtensionFuncPtr(glGenQueries, "glGenQueries", "glGenQueriesARB");
    setGLExtensionFuncPtr(glDeleteQueries, "glDeleteQueries", "glDeleteQueriesARB");
    setGLExtensionFuncPtr(glIsQuery, "glIsQuery", "glIsQueryARB");
    setGLExtensionFuncPtr(glBeginQuery, "glBeginQuery", "glBeginQueryARB");
    setGLExtensionFuncPtr(glEndQuery, "glEndQuery", "glEndQueryARB");
    setGLExtensionFuncPtr(glGetQueryiv, "glGetQueryiv", "glGetQueryivARB");
    setGLExtensionFuncPtr(glGetQueryObjectiv, "glGetQueryObjectiv","glGetQueryObjectivARB");
    setGLExtensionFuncPtr(glGetQueryObjectuiv, "glGetQueryObjectuiv","glGetQueryObjectuivARB");
    setGLExtensionFuncPtr(glGetQueryObjectui64v, "glGetQueryObjectui64v","glGetQueryObjectui64vEXT");
    setGLExtensionFuncPtr(glQueryCounter, "glQueryCounter");
    setGLExtensionFuncPtr(glGetInteger64v, "glGetInteger64v");

}



///////////////////////////////////////////////////////////////////////////
// C++-friendly convenience methods

GLuint GL2Extensions::getCurrentProgram() const
{
    if( glVersion >= 2.0f )
    {
        // GLSL as GL v2.0 core functionality
        GLint result = 0;
        glGetIntegerv( GL_CURRENT_PROGRAM, &result );
        return static_cast<GLuint>(result);
    }
    else if (glGetHandleARB)
    {
        // fallback for GLSL as GL v1.5 ARB extension
#ifndef GL_PROGRAM_OBJECT_ARB
#define GL_PROGRAM_OBJECT_ARB 0x8B40
#endif
        return glGetHandleARB( GL_PROGRAM_OBJECT_ARB );
    }
    else
    {
        OSG_WARN<<"Warning GL2Extensions::getCurrentProgram not supported"<<std::endl;;
        return 0;
    }
}


bool GL2Extensions::getProgramInfoLog( GLuint program, std::string& result ) const
{
    GLsizei bufLen = 0;        // length of buffer to allocate
    GLsizei strLen = 0;        // strlen GL actually wrote to buffer

    glGetProgramiv( program, GL_INFO_LOG_LENGTH, &bufLen );
    if( bufLen > 1 )
    {
        GLchar* infoLog = new GLchar[bufLen];
        glGetProgramInfoLog( program, bufLen, &strLen, infoLog );
        if( strLen > 0 ) result = reinterpret_cast<char*>(infoLog);
        delete [] infoLog;
    }
    return (strLen > 0);
}


bool GL2Extensions::getShaderInfoLog( GLuint shader, std::string& result ) const
{
    GLsizei bufLen = 0;        // length of buffer to allocate
    GLsizei strLen = 0;        // strlen GL actually wrote to buffer

    glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &bufLen );
    if( bufLen > 1 )
    {
        GLchar* infoLog = new GLchar[bufLen];
        glGetShaderInfoLog( shader, bufLen, &strLen, infoLog );
        if( strLen > 0 ) result = reinterpret_cast<char*>(infoLog);
        delete [] infoLog;
    }
    return (strLen > 0);
}


bool GL2Extensions::getAttribLocation( const char* attribName, GLuint& location ) const
{
    // is there an active GLSL program?
    GLuint program = getCurrentProgram();
    if( glIsProgram(program) == GL_FALSE ) return false;

    // has that program been successfully linked?
    GLint linked = GL_FALSE;
    glGetProgramiv( program, GL_LINK_STATUS, &linked );
    if( linked == GL_FALSE ) return false;

    // is there such a named attribute?
    GLint loc = glGetAttribLocation( program, reinterpret_cast<const GLchar*>(attribName) );
    if( loc < 0 ) return false;

    location = loc;
    return true;
}


bool GL2Extensions::getFragDataLocation( const char* fragDataName, GLuint& location ) const
{
    // is there an active GLSL program?
    GLuint program = getCurrentProgram();
    if( glIsProgram(program) == GL_FALSE ) return false;

    // has that program been successfully linked?
    GLint linked = GL_FALSE;
    glGetProgramiv( program, GL_LINK_STATUS, &linked );
    if( linked == GL_FALSE ) return false;

    // check if supported
    if (glGetFragDataLocation == NULL) return false;

    // is there such a named attribute?
    GLint loc = glGetFragDataLocation( program, reinterpret_cast<const GLchar*>(fragDataName) );
    if( loc < 0 ) return false;

    location = loc;
    return true;
}

