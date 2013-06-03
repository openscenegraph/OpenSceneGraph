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
// Extension function pointers for OpenGL v2.x

GL2Extensions::GL2Extensions(unsigned int contextID)
{
    setupGL2Extensions(contextID);
}


GL2Extensions::GL2Extensions(const GL2Extensions& rhs) : osg::Referenced()
{
    _glVersion = rhs._glVersion;
    _glslLanguageVersion = rhs._glslLanguageVersion;

    _isShaderObjectsSupported = rhs._isShaderObjectsSupported;
    _isVertexShaderSupported = rhs._isVertexShaderSupported;
    _isFragmentShaderSupported = rhs._isFragmentShaderSupported;
    _isLanguage100Supported = rhs._isLanguage100Supported;
    _isGeometryShader4Supported = rhs._isGeometryShader4Supported;
    _areTessellationShadersSupported = rhs._areTessellationShadersSupported;
    _isGpuShader4Supported = rhs._isGpuShader4Supported;
    _isUniformBufferObjectSupported = rhs._isUniformBufferObjectSupported;
    _isGetProgramBinarySupported = rhs._isGetProgramBinarySupported;
    _isGpuShaderFp64Supported = rhs._isGpuShaderFp64Supported;
    _isShaderAtomicCountersSupported = rhs._isShaderAtomicCountersSupported;

    _glBlendEquationSeparate = rhs._glBlendEquationSeparate;
    _glDrawBuffers = rhs._glDrawBuffers;
    _glStencilOpSeparate = rhs._glStencilOpSeparate;
    _glStencilFuncSeparate = rhs._glStencilFuncSeparate;
    _glStencilMaskSeparate = rhs._glStencilMaskSeparate;
    _glAttachShader = rhs._glAttachShader;
    _glBindAttribLocation = rhs._glBindAttribLocation;
    _glCompileShader = rhs._glCompileShader;
    _glCreateProgram = rhs._glCreateProgram;
    _glCreateShader = rhs._glCreateShader;
    _glDeleteProgram = rhs._glDeleteProgram;
    _glDeleteShader = rhs._glDeleteShader;
    _glDetachShader = rhs._glDetachShader;
    _glDisableVertexAttribArray = rhs._glDisableVertexAttribArray;
    _glEnableVertexAttribArray = rhs._glEnableVertexAttribArray;
    _glGetActiveAttrib = rhs._glGetActiveAttrib;
    _glGetActiveUniform = rhs._glGetActiveUniform;
    _glGetAttachedShaders = rhs._glGetAttachedShaders;
    _glGetAttribLocation = rhs._glGetAttribLocation;
    _glGetProgramiv = rhs._glGetProgramiv;
    _glGetProgramInfoLog = rhs._glGetProgramInfoLog;
    _glGetShaderiv = rhs._glGetShaderiv;
    _glGetShaderInfoLog = rhs._glGetShaderInfoLog;
    _glGetShaderSource = rhs._glGetShaderSource;
    _glGetUniformLocation = rhs._glGetUniformLocation;
    _glGetUniformfv = rhs._glGetUniformfv;
    _glGetUniformiv = rhs._glGetUniformiv;
    _glGetVertexAttribdv = rhs._glGetVertexAttribdv;
    _glGetVertexAttribfv = rhs._glGetVertexAttribfv;
    _glGetVertexAttribiv = rhs._glGetVertexAttribiv;
    _glGetVertexAttribPointerv = rhs._glGetVertexAttribPointerv;
    _glIsProgram = rhs._glIsProgram;
    _glIsShader = rhs._glIsShader;
    _glLinkProgram = rhs._glLinkProgram;
    _glShaderSource = rhs._glShaderSource;
    _glUseProgram = rhs._glUseProgram;
    _glUniform1f = rhs._glUniform1f;
    _glUniform2f = rhs._glUniform2f;
    _glUniform3f = rhs._glUniform3f;
    _glUniform4f = rhs._glUniform4f;
    _glUniform1i = rhs._glUniform1i;
    _glUniform2i = rhs._glUniform2i;
    _glUniform3i = rhs._glUniform3i;
    _glUniform4i = rhs._glUniform4i;
    _glUniform1fv = rhs._glUniform1fv;
    _glUniform2fv = rhs._glUniform2fv;
    _glUniform3fv = rhs._glUniform3fv;
    _glUniform4fv = rhs._glUniform4fv;
    _glUniform1iv = rhs._glUniform1iv;
    _glUniform2iv = rhs._glUniform2iv;
    _glUniform3iv = rhs._glUniform3iv;
    _glUniform4iv = rhs._glUniform4iv;
    _glUniformMatrix2fv = rhs._glUniformMatrix2fv;
    _glUniformMatrix3fv = rhs._glUniformMatrix3fv;
    _glUniformMatrix4fv = rhs._glUniformMatrix4fv;
    _glValidateProgram = rhs._glValidateProgram;
    _glVertexAttrib1d = rhs._glVertexAttrib1d;
    _glVertexAttrib1dv = rhs._glVertexAttrib1dv;
    _glVertexAttrib1f = rhs._glVertexAttrib1f;
    _glVertexAttrib1fv = rhs._glVertexAttrib1fv;
    _glVertexAttrib1s = rhs._glVertexAttrib1s;
    _glVertexAttrib1sv = rhs._glVertexAttrib1sv;
    _glVertexAttrib2d = rhs._glVertexAttrib2d;
    _glVertexAttrib2dv = rhs._glVertexAttrib2dv;
    _glVertexAttrib2f = rhs._glVertexAttrib2f;
    _glVertexAttrib2fv = rhs._glVertexAttrib2fv;
    _glVertexAttrib2s = rhs._glVertexAttrib2s;
    _glVertexAttrib2sv = rhs._glVertexAttrib2sv;
    _glVertexAttrib3d = rhs._glVertexAttrib3d;
    _glVertexAttrib3dv = rhs._glVertexAttrib3dv;
    _glVertexAttrib3f = rhs._glVertexAttrib3f;
    _glVertexAttrib3fv = rhs._glVertexAttrib3fv;
    _glVertexAttrib3s = rhs._glVertexAttrib3s;
    _glVertexAttrib3sv = rhs._glVertexAttrib3sv;
    _glVertexAttrib4Nbv = rhs._glVertexAttrib4Nbv;
    _glVertexAttrib4Niv = rhs._glVertexAttrib4Niv;
    _glVertexAttrib4Nsv = rhs._glVertexAttrib4Nsv;
    _glVertexAttrib4Nub = rhs._glVertexAttrib4Nub;
    _glVertexAttrib4Nubv = rhs._glVertexAttrib4Nubv;
    _glVertexAttrib4Nuiv = rhs._glVertexAttrib4Nuiv;
    _glVertexAttrib4Nusv = rhs._glVertexAttrib4Nusv;
    _glVertexAttrib4bv = rhs._glVertexAttrib4bv;
    _glVertexAttrib4d = rhs._glVertexAttrib4d;
    _glVertexAttrib4dv = rhs._glVertexAttrib4dv;
    _glVertexAttrib4f = rhs._glVertexAttrib4f;
    _glVertexAttrib4fv = rhs._glVertexAttrib4fv;
    _glVertexAttrib4iv = rhs._glVertexAttrib4iv;
    _glVertexAttrib4s = rhs._glVertexAttrib4s;
    _glVertexAttrib4sv = rhs._glVertexAttrib4sv;
    _glVertexAttrib4ubv = rhs._glVertexAttrib4ubv;
    _glVertexAttrib4uiv = rhs._glVertexAttrib4uiv;
    _glVertexAttrib4usv = rhs._glVertexAttrib4usv;
    _glVertexAttribPointer = rhs._glVertexAttribPointer;
    _glVertexAttribDivisor = rhs._glVertexAttribDivisor;

    _glGetInfoLogARB = rhs._glGetInfoLogARB;
    _glGetObjectParameterivARB = rhs._glGetObjectParameterivARB;
    _glDeleteObjectARB = rhs._glDeleteObjectARB;
    _glGetHandleARB = rhs._glGetHandleARB;

    // GL 2.1
    _glUniformMatrix2x3fv = rhs._glUniformMatrix2x3fv;
    _glUniformMatrix3x2fv = rhs._glUniformMatrix3x2fv;
    _glUniformMatrix2x4fv = rhs._glUniformMatrix2x4fv;
    _glUniformMatrix4x2fv = rhs._glUniformMatrix4x2fv;
    _glUniformMatrix3x4fv = rhs._glUniformMatrix3x4fv;
    _glUniformMatrix4x3fv = rhs._glUniformMatrix4x3fv;

    // EXT_geometry_shader4
    _glProgramParameteri = rhs._glProgramParameteri;
    _glFramebufferTexture = rhs._glFramebufferTexture;
    _glFramebufferTextureLayer = rhs._glFramebufferTextureLayer;
    _glFramebufferTextureFace = rhs._glFramebufferTextureFace;

    // EXT_gpu_shader4
    _glGetUniformuiv = rhs._glGetUniformuiv;
    _glBindFragDataLocation = rhs._glBindFragDataLocation;
    _glGetFragDataLocation = rhs._glGetFragDataLocation;
    _glUniform1ui = rhs._glUniform1ui;
    _glUniform2ui = rhs._glUniform2ui;
    _glUniform3ui = rhs._glUniform3ui;
    _glUniform4ui = rhs._glUniform4ui;
    _glUniform1uiv = rhs._glUniform1uiv;
    _glUniform2uiv = rhs._glUniform2uiv;
    _glUniform3uiv = rhs._glUniform3uiv;
    _glUniform4uiv = rhs._glUniform4uiv;

    // ARB_uniform_buffer_object
    _glGetUniformIndices = rhs._glGetUniformIndices;
    _glGetActiveUniformsiv = rhs._glGetActiveUniformsiv;
    _glGetActiveUniformName = rhs._glGetActiveUniformName;
    _glGetUniformBlockIndex = rhs._glGetUniformBlockIndex;
    _glGetActiveUniformBlockiv = rhs._glGetActiveUniformBlockiv;
    _glGetActiveUniformBlockName = rhs._glGetActiveUniformBlockName;
    _glUniformBlockBinding = rhs._glUniformBlockBinding;

    // ARB_get_program_binary
    _glGetProgramBinary = rhs._glGetProgramBinary;
    _glProgramBinary = rhs._glProgramBinary;

    // ARB_gpu_shader_fp64
    _glUniform1d = rhs._glUniform1d;
    _glUniform2d = rhs._glUniform2d;
    _glUniform3d = rhs._glUniform3d;
    _glUniform4d = rhs._glUniform4d;
    _glUniform1dv = rhs._glUniform1dv;
    _glUniform2dv = rhs._glUniform2dv;
    _glUniform3dv = rhs._glUniform3dv;
    _glUniform4dv = rhs._glUniform4dv;
    _glUniformMatrix2dv = rhs._glUniformMatrix2dv;
    _glUniformMatrix3dv = rhs._glUniformMatrix3dv;
    _glUniformMatrix4dv = rhs._glUniformMatrix4dv;
    _glUniformMatrix2x3dv = rhs._glUniformMatrix2x3dv;
    _glUniformMatrix3x2dv = rhs._glUniformMatrix3x2dv;
    _glUniformMatrix2x4dv = rhs._glUniformMatrix2x4dv;
    _glUniformMatrix4x2dv = rhs._glUniformMatrix4x2dv;
    _glUniformMatrix3x4dv = rhs._glUniformMatrix3x4dv;
    _glUniformMatrix4x3dv = rhs._glUniformMatrix4x3dv;

    // ARB_shader_atomic_counters
    _glGetActiveAtomicCounterBufferiv = rhs._glGetActiveAtomicCounterBufferiv;

    // ARB_compute_shader
    _glDispatchCompute = rhs._glDispatchCompute;
}


void GL2Extensions::lowestCommonDenominator(const GL2Extensions& rhs)
{
    if (rhs._glVersion < _glVersion) _glVersion = rhs._glVersion;
    if (rhs._glslLanguageVersion < _glslLanguageVersion)
               _glslLanguageVersion = rhs._glslLanguageVersion;

    if (!rhs._isShaderObjectsSupported) _isShaderObjectsSupported = false;
    if (!rhs._isVertexShaderSupported) _isVertexShaderSupported = false;
    if (!rhs._isFragmentShaderSupported) _isFragmentShaderSupported = false;
    if (!rhs._isLanguage100Supported) _isLanguage100Supported = false;
    if (!rhs._isGeometryShader4Supported) _isGeometryShader4Supported = false;
    if (!rhs._areTessellationShadersSupported) _areTessellationShadersSupported = false;
    if (!rhs._isGpuShader4Supported) _isGpuShader4Supported = false;
    if (!rhs._isUniformBufferObjectSupported) _isUniformBufferObjectSupported = false;
    if (!rhs._isGetProgramBinarySupported) _isGetProgramBinarySupported = false;
    if (!rhs._isGpuShaderFp64Supported) _isGpuShaderFp64Supported = false;
    if (!rhs._isShaderAtomicCountersSupported) _isShaderAtomicCountersSupported = false;

    if (!rhs._glBlendEquationSeparate) _glBlendEquationSeparate = 0;
    if (!rhs._glDrawBuffers) _glDrawBuffers = 0;
    if (!rhs._glStencilOpSeparate) _glStencilOpSeparate = 0;
    if (!rhs._glStencilFuncSeparate) _glStencilFuncSeparate = 0;
    if (!rhs._glStencilMaskSeparate) _glStencilMaskSeparate = 0;
    if (!rhs._glAttachShader) _glAttachShader = 0;
    if (!rhs._glBindAttribLocation) _glBindAttribLocation = 0;
    if (!rhs._glCompileShader) _glCompileShader = 0;
    if (!rhs._glCreateProgram) _glCreateProgram = 0;
    if (!rhs._glCreateShader) _glCreateShader = 0;
    if (!rhs._glDeleteProgram) _glDeleteProgram = 0;
    if (!rhs._glDeleteShader) _glDeleteShader = 0;
    if (!rhs._glDetachShader) _glDetachShader = 0;
    if (!rhs._glDisableVertexAttribArray) _glDisableVertexAttribArray = 0;
    if (!rhs._glEnableVertexAttribArray) _glEnableVertexAttribArray = 0;
    if (!rhs._glGetActiveAttrib) _glGetActiveAttrib = 0;
    if (!rhs._glGetActiveUniform) _glGetActiveUniform = 0;
    if (!rhs._glGetAttachedShaders) _glGetAttachedShaders = 0;
    if (!rhs._glGetAttribLocation) _glGetAttribLocation = 0;
    if (!rhs._glGetProgramiv) _glGetProgramiv = 0;
    if (!rhs._glGetProgramInfoLog) _glGetProgramInfoLog = 0;
    if (!rhs._glGetShaderiv) _glGetShaderiv = 0;
    if (!rhs._glGetShaderInfoLog) _glGetShaderInfoLog = 0;
    if (!rhs._glGetShaderSource) _glGetShaderSource = 0;
    if (!rhs._glGetUniformLocation) _glGetUniformLocation = 0;
    if (!rhs._glGetUniformfv) _glGetUniformfv = 0;
    if (!rhs._glGetUniformiv) _glGetUniformiv = 0;
    if (!rhs._glGetVertexAttribdv) _glGetVertexAttribdv = 0;
    if (!rhs._glGetVertexAttribfv) _glGetVertexAttribfv = 0;
    if (!rhs._glGetVertexAttribiv) _glGetVertexAttribiv = 0;
    if (!rhs._glGetVertexAttribPointerv) _glGetVertexAttribPointerv = 0;
    if (!rhs._glIsProgram) _glIsProgram = 0;
    if (!rhs._glIsShader) _glIsShader = 0;
    if (!rhs._glLinkProgram) _glLinkProgram = 0;
    if (!rhs._glShaderSource) _glShaderSource = 0;
    if (!rhs._glUseProgram) _glUseProgram = 0;
    if (!rhs._glUniform1f) _glUniform1f = 0;
    if (!rhs._glUniform2f) _glUniform2f = 0;
    if (!rhs._glUniform3f) _glUniform3f = 0;
    if (!rhs._glUniform4f) _glUniform4f = 0;
    if (!rhs._glUniform1i) _glUniform1i = 0;
    if (!rhs._glUniform2i) _glUniform2i = 0;
    if (!rhs._glUniform3i) _glUniform3i = 0;
    if (!rhs._glUniform4i) _glUniform4i = 0;
    if (!rhs._glUniform1fv) _glUniform1fv = 0;
    if (!rhs._glUniform2fv) _glUniform2fv = 0;
    if (!rhs._glUniform3fv) _glUniform3fv = 0;
    if (!rhs._glUniform4fv) _glUniform4fv = 0;
    if (!rhs._glUniform1iv) _glUniform1iv = 0;
    if (!rhs._glUniform2iv) _glUniform2iv = 0;
    if (!rhs._glUniform3iv) _glUniform3iv = 0;
    if (!rhs._glUniform4iv) _glUniform4iv = 0;
    if (!rhs._glUniformMatrix2fv) _glUniformMatrix2fv = 0;
    if (!rhs._glUniformMatrix3fv) _glUniformMatrix3fv = 0;
    if (!rhs._glUniformMatrix4fv) _glUniformMatrix4fv = 0;
    if (!rhs._glValidateProgram) _glValidateProgram = 0;
    if (!rhs._glVertexAttrib1d) _glVertexAttrib1d = 0;
    if (!rhs._glVertexAttrib1dv) _glVertexAttrib1dv = 0;
    if (!rhs._glVertexAttrib1f) _glVertexAttrib1f = 0;
    if (!rhs._glVertexAttrib1fv) _glVertexAttrib1fv = 0;
    if (!rhs._glVertexAttrib1s) _glVertexAttrib1s = 0;
    if (!rhs._glVertexAttrib1sv) _glVertexAttrib1sv = 0;
    if (!rhs._glVertexAttrib2d) _glVertexAttrib2d = 0;
    if (!rhs._glVertexAttrib2dv) _glVertexAttrib2dv = 0;
    if (!rhs._glVertexAttrib2f) _glVertexAttrib2f = 0;
    if (!rhs._glVertexAttrib2fv) _glVertexAttrib2fv = 0;
    if (!rhs._glVertexAttrib2s) _glVertexAttrib2s = 0;
    if (!rhs._glVertexAttrib2sv) _glVertexAttrib2sv = 0;
    if (!rhs._glVertexAttrib3d) _glVertexAttrib3d = 0;
    if (!rhs._glVertexAttrib3dv) _glVertexAttrib3dv = 0;
    if (!rhs._glVertexAttrib3f) _glVertexAttrib3f = 0;
    if (!rhs._glVertexAttrib3fv) _glVertexAttrib3fv = 0;
    if (!rhs._glVertexAttrib3s) _glVertexAttrib3s = 0;
    if (!rhs._glVertexAttrib3sv) _glVertexAttrib3sv = 0;
    if (!rhs._glVertexAttrib4Nbv) _glVertexAttrib4Nbv = 0;
    if (!rhs._glVertexAttrib4Niv) _glVertexAttrib4Niv = 0;
    if (!rhs._glVertexAttrib4Nsv) _glVertexAttrib4Nsv = 0;
    if (!rhs._glVertexAttrib4Nub) _glVertexAttrib4Nub = 0;
    if (!rhs._glVertexAttrib4Nubv) _glVertexAttrib4Nubv = 0;
    if (!rhs._glVertexAttrib4Nuiv) _glVertexAttrib4Nuiv = 0;
    if (!rhs._glVertexAttrib4Nusv) _glVertexAttrib4Nusv = 0;
    if (!rhs._glVertexAttrib4bv) _glVertexAttrib4bv = 0;
    if (!rhs._glVertexAttrib4d) _glVertexAttrib4d = 0;
    if (!rhs._glVertexAttrib4dv) _glVertexAttrib4dv = 0;
    if (!rhs._glVertexAttrib4f) _glVertexAttrib4f = 0;
    if (!rhs._glVertexAttrib4fv) _glVertexAttrib4fv = 0;
    if (!rhs._glVertexAttrib4iv) _glVertexAttrib4iv = 0;
    if (!rhs._glVertexAttrib4s) _glVertexAttrib4s = 0;
    if (!rhs._glVertexAttrib4sv) _glVertexAttrib4sv = 0;
    if (!rhs._glVertexAttrib4ubv) _glVertexAttrib4ubv = 0;
    if (!rhs._glVertexAttrib4uiv) _glVertexAttrib4uiv = 0;
    if (!rhs._glVertexAttrib4usv) _glVertexAttrib4usv = 0;
    if (!rhs._glVertexAttribPointer) _glVertexAttribPointer = 0;
    if (!rhs._glVertexAttribDivisor) _glVertexAttribDivisor = 0;

    if (!rhs._glGetInfoLogARB) _glGetInfoLogARB = 0;
    if (!rhs._glGetObjectParameterivARB) _glGetObjectParameterivARB = 0;
    if (!rhs._glDeleteObjectARB) _glDeleteObjectARB = 0;
    if (!rhs._glGetHandleARB) _glGetHandleARB = 0;

    // GL 2.1
    if (!rhs._glUniformMatrix2x3fv) _glUniformMatrix2x3fv = 0;
    if (!rhs._glUniformMatrix3x2fv) _glUniformMatrix3x2fv = 0;
    if (!rhs._glUniformMatrix2x4fv) _glUniformMatrix2x4fv = 0;
    if (!rhs._glUniformMatrix4x2fv) _glUniformMatrix4x2fv = 0;
    if (!rhs._glUniformMatrix3x4fv) _glUniformMatrix3x4fv = 0;
    if (!rhs._glUniformMatrix4x3fv) _glUniformMatrix4x3fv = 0;

    // EXT_geometry_shader4
    if (!rhs._glProgramParameteri) _glProgramParameteri = 0;
    if (!rhs._glFramebufferTexture) _glFramebufferTexture = 0;
    if (!rhs._glFramebufferTextureLayer) _glFramebufferTextureLayer = 0;
    if (!rhs._glFramebufferTextureFace) _glFramebufferTextureFace = 0;

    // ARB_tessellation_shader
    if (!rhs._glPatchParameteri) _glPatchParameteri = 0;
    if (!rhs._glPatchParameterfv) _glPatchParameterfv = 0;

    // EXT_gpu_shader4
    if (!rhs._glGetUniformuiv) _glGetUniformuiv = 0;
    if (!rhs._glBindFragDataLocation) _glBindFragDataLocation = 0;
    if (!rhs._glGetFragDataLocation) _glGetFragDataLocation = 0;
    if (!rhs._glUniform1ui) _glUniform1ui = 0;
    if (!rhs._glUniform2ui) _glUniform2ui = 0;
    if (!rhs._glUniform3ui) _glUniform3ui = 0;
    if (!rhs._glUniform4ui) _glUniform4ui = 0;
    if (!rhs._glUniform1uiv) _glUniform1uiv = 0;
    if (!rhs._glUniform2uiv) _glUniform2uiv = 0;
    if (!rhs._glUniform3uiv) _glUniform3uiv = 0;
    if (!rhs._glUniform4uiv) _glUniform4uiv = 0;

    // ARB_uniform_buffer_object
    if (!rhs._glGetUniformIndices) _glGetUniformIndices = 0;
    if (!rhs._glGetActiveUniformsiv) _glGetActiveUniformsiv = 0;
    if (!rhs._glGetActiveUniformName) _glGetActiveUniformName = 0;
    if (!rhs._glGetUniformBlockIndex) _glGetUniformBlockIndex = 0;
    if (!rhs._glGetActiveUniformBlockiv) _glGetActiveUniformBlockiv = 0;
    if (!rhs._glGetActiveUniformBlockName) _glGetActiveUniformBlockName = 0;
    if (!rhs._glUniformBlockBinding) _glUniformBlockBinding = 0;

    // ARB_get_program_binary
    if (!rhs._glGetProgramBinary) _glGetProgramBinary = 0;
    if (!rhs._glProgramBinary) _glProgramBinary = 0;

    // ARB_gpu_shader_fp64
    if(!rhs._glUniform1d) _glUniform1d = 0;
    if(!rhs._glUniform2d) _glUniform2d = 0;
    if(!rhs._glUniform3d) _glUniform3d = 0;
    if(!rhs._glUniform4d) _glUniform4d = 0;
    if(!rhs._glUniform1dv) _glUniform1dv = 0;
    if(!rhs._glUniform2dv) _glUniform2dv = 0;
    if(!rhs._glUniform3dv) _glUniform3dv = 0;
    if(!rhs._glUniform4dv) _glUniform4dv = 0;
    if(!rhs._glUniformMatrix2dv) _glUniformMatrix2dv = 0;
    if(!rhs._glUniformMatrix3dv) _glUniformMatrix3dv = 0;
    if(!rhs._glUniformMatrix4dv) _glUniformMatrix4dv = 0;
    if(!rhs._glUniformMatrix2x3dv) _glUniformMatrix2x3dv = 0;
    if(!rhs._glUniformMatrix3x2dv) _glUniformMatrix3x2dv = 0;
    if(!rhs._glUniformMatrix2x4dv) _glUniformMatrix2x4dv = 0;
    if(!rhs._glUniformMatrix4x2dv) _glUniformMatrix4x2dv = 0;
    if(!rhs._glUniformMatrix3x4dv) _glUniformMatrix3x4dv = 0;
    if(!rhs._glUniformMatrix4x3dv) _glUniformMatrix4x3dv = 0;

    // ARB_shader_atomic_counters
    if(!rhs._glGetActiveAtomicCounterBufferiv) _glGetActiveAtomicCounterBufferiv = 0;

    // ARB_compute_shder
    if(!rhs._glDispatchCompute) _glDispatchCompute = 0;
}


void GL2Extensions::setupGL2Extensions(unsigned int contextID)
{
    const char* version = (const char*) glGetString( GL_VERSION );
    if (!version)
    {
        OSG_NOTIFY(osg::FATAL)<<"Error: OpenGL version test failed, requires valid graphics context."<<std::endl;

        _glVersion = 0.0f;
        _glslLanguageVersion = 0.0f;

        _isShaderObjectsSupported = false;
        _isVertexShaderSupported = false;
        _isFragmentShaderSupported = false;
        _isLanguage100Supported = false;
        _isGeometryShader4Supported = false;
        _areTessellationShadersSupported = false;
        _isGpuShader4Supported = false;
        _isUniformBufferObjectSupported = false;
        _isGetProgramBinarySupported = false;
        _isGpuShaderFp64Supported = false;
        _isShaderAtomicCountersSupported = false;

        _glBlendEquationSeparate= 0;
        _glDrawBuffers= 0;
        _glStencilOpSeparate= 0;
        _glStencilFuncSeparate= 0;
        _glStencilMaskSeparate= 0;
        _glAttachShader= 0;
        _glBindAttribLocation= 0;
        _glCompileShader= 0;
        _glCreateProgram= 0;
        _glCreateShader= 0;
        _glDeleteProgram= 0;
        _glDeleteShader= 0;
        _glDetachShader= 0;
        _glDisableVertexAttribArray= 0;
        _glEnableVertexAttribArray= 0;
        _glGetActiveAttrib= 0;
        _glGetActiveUniform= 0;
        _glGetAttachedShaders= 0;
        _glGetAttribLocation= 0;
        _glGetProgramiv= 0;
        _glGetProgramInfoLog= 0;
        _glGetShaderiv= 0;
        _glGetShaderInfoLog= 0;
        _glGetShaderSource= 0;
        _glGetUniformLocation= 0;
        _glGetUniformfv= 0;
        _glGetUniformiv= 0;
        _glGetVertexAttribdv= 0;
        _glGetVertexAttribfv= 0;
        _glGetVertexAttribiv= 0;
        _glGetVertexAttribPointerv= 0;
        _glIsProgram= 0;
        _glIsShader= 0;
        _glLinkProgram= 0;
        _glShaderSource= 0;
        _glUseProgram= 0;
        _glUniform1f= 0;
        _glUniform2f= 0;
        _glUniform3f= 0;
        _glUniform4f= 0;
        _glUniform1i= 0;
        _glUniform2i= 0;
        _glUniform3i= 0;
        _glUniform4i= 0;
        _glUniform1fv= 0;
        _glUniform2fv= 0;
        _glUniform3fv= 0;
        _glUniform4fv= 0;
        _glUniform1iv= 0;
        _glUniform2iv= 0;
        _glUniform3iv= 0;
        _glUniform4iv= 0;
        _glUniformMatrix2fv= 0;
        _glUniformMatrix3fv= 0;
        _glUniformMatrix4fv= 0;
        _glValidateProgram= 0;
        _glVertexAttrib1d= 0;
        _glVertexAttrib1dv= 0;
        _glVertexAttrib1f= 0;
        _glVertexAttrib1fv= 0;
        _glVertexAttrib1s= 0;
        _glVertexAttrib1sv= 0;
        _glVertexAttrib2d= 0;
        _glVertexAttrib2dv= 0;
        _glVertexAttrib2f= 0;
        _glVertexAttrib2fv= 0;
        _glVertexAttrib2s= 0;
        _glVertexAttrib2sv= 0;
        _glVertexAttrib3d= 0;
        _glVertexAttrib3dv= 0;
        _glVertexAttrib3f= 0;
        _glVertexAttrib3fv= 0;
        _glVertexAttrib3s= 0;
        _glVertexAttrib3sv= 0;
        _glVertexAttrib4Nbv= 0;
        _glVertexAttrib4Niv= 0;
        _glVertexAttrib4Nsv= 0;
        _glVertexAttrib4Nub= 0;
        _glVertexAttrib4Nubv= 0;
        _glVertexAttrib4Nuiv= 0;
        _glVertexAttrib4Nusv= 0;
        _glVertexAttrib4bv= 0;
        _glVertexAttrib4d= 0;
        _glVertexAttrib4dv= 0;
        _glVertexAttrib4f= 0;
        _glVertexAttrib4fv= 0;
        _glVertexAttrib4iv= 0;
        _glVertexAttrib4s= 0;
        _glVertexAttrib4sv= 0;
        _glVertexAttrib4ubv= 0;
        _glVertexAttrib4uiv= 0;
        _glVertexAttrib4usv= 0;
        _glVertexAttribPointer= 0;
        _glVertexAttribDivisor= 0;

        _glGetInfoLogARB= 0;
        _glGetObjectParameterivARB= 0;
        _glDeleteObjectARB= 0;
        _glGetHandleARB= 0;

        // GL 2.1
        _glUniformMatrix2x3fv= 0;
        _glUniformMatrix3x2fv= 0;
        _glUniformMatrix2x4fv= 0;
        _glUniformMatrix4x2fv= 0;
        _glUniformMatrix3x4fv= 0;
        _glUniformMatrix4x3fv= 0;

        // EXT_geometry_shader4
        _glProgramParameteri= 0;
        _glFramebufferTexture= 0;
        _glFramebufferTextureLayer= 0;
        _glFramebufferTextureFace= 0;

        // ARB_tesselation_shader
        _glPatchParameteri= 0;
        _glPatchParameterfv= 0;

        // EXT_gpu_shader4
        _glGetUniformuiv= 0;
        _glBindFragDataLocation= 0;
        _glGetFragDataLocation= 0;
        _glUniform1ui= 0;
        _glUniform2ui= 0;
        _glUniform3ui= 0;
        _glUniform4ui= 0;
        _glUniform1uiv= 0;
        _glUniform2uiv= 0;
        _glUniform3uiv= 0;
        _glUniform4uiv= 0;

        // ARB_uniform_buffer_object
        _glGetUniformIndices= 0;
        _glGetActiveUniformsiv= 0;
        _glGetActiveUniformName= 0;
        _glGetUniformBlockIndex= 0;
        _glGetActiveUniformBlockiv= 0;
        _glGetActiveUniformBlockName= 0;
        _glUniformBlockBinding= 0;

        // ARB_get_program_binary
        _glGetProgramBinary= 0;
        _glProgramBinary= 0;

        // ARB_gpu_shader_fp64
        _glUniform1d= 0;
        _glUniform2d= 0;
        _glUniform3d= 0;
        _glUniform4d= 0;
        _glUniform1dv= 0;
        _glUniform2dv= 0;
        _glUniform3dv= 0;
        _glUniform4dv= 0;
        _glUniformMatrix2dv= 0;
        _glUniformMatrix3dv= 0;
        _glUniformMatrix4dv= 0;
        _glUniformMatrix2x3dv= 0;
        _glUniformMatrix3x2dv= 0;
        _glUniformMatrix2x4dv= 0;
        _glUniformMatrix4x2dv= 0;
        _glUniformMatrix3x4dv= 0;
        _glUniformMatrix4x3dv= 0;

        // ARB_shader_atomic_counters
        _glGetActiveAtomicCounterBufferiv= 0;

        // ARB_compute_shader
        _glDispatchCompute= 0;

        return;
    }

    _glVersion = findAsciiToFloat( version );
    _glslLanguageVersion = 0.0f;

    bool shadersBuiltIn = OSG_GLES2_FEATURES || OSG_GL3_FEATURES;

    _isShaderObjectsSupported = shadersBuiltIn || osg::isGLExtensionSupported(contextID,"GL_ARB_shader_objects");
    _isVertexShaderSupported = shadersBuiltIn || osg::isGLExtensionSupported(contextID,"GL_ARB_vertex_shader");
    _isFragmentShaderSupported = shadersBuiltIn || osg::isGLExtensionSupported(contextID,"GL_ARB_fragment_shader");
    _isLanguage100Supported = shadersBuiltIn || osg::isGLExtensionSupported(contextID,"GL_ARB_shading_language_100");
    _isGeometryShader4Supported = osg::isGLExtensionSupported(contextID,"GL_EXT_geometry_shader4");
    _isGpuShader4Supported = osg::isGLExtensionSupported(contextID,"GL_EXT_gpu_shader4");
    _areTessellationShadersSupported = osg::isGLExtensionSupported(contextID, "GL_ARB_tessellation_shader");
    _isUniformBufferObjectSupported = osg::isGLExtensionSupported(contextID,"GL_ARB_uniform_buffer_object");
    _isGetProgramBinarySupported = osg::isGLExtensionSupported(contextID,"GL_ARB_get_program_binary");
    _isGpuShaderFp64Supported = osg::isGLExtensionSupported(contextID,"GL_ARB_gpu_shader_fp64");
    _isShaderAtomicCountersSupported = osg::isGLExtensionSupported(contextID,"GL_ARB_shader_atomic_counters");

    if( isGlslSupported() )
    {
        // If glGetString raises an error, assume initial release "1.00"
        while(glGetError() != GL_NO_ERROR) {}        // reset error flag

        const char* langVerStr = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
        if( (glGetError() == GL_NO_ERROR) && langVerStr )
        {
            _glslLanguageVersion = (findAsciiToFloat(langVerStr));
        }
        else
            _glslLanguageVersion = 1.0f;
    }

    OSG_INFO
            << "glVersion=" << getGlVersion() << ", "
            << "isGlslSupported=" << (isGlslSupported() ? "YES" : "NO") << ", "
            << "glslLanguageVersion=" << getLanguageVersion()
            << std::endl;


    setGLExtensionFuncPtr(_glBlendEquationSeparate, "glBlendEquationSeparate");
    setGLExtensionFuncPtr(_glDrawBuffers, "glDrawBuffers", "glDrawBuffersARB");
    setGLExtensionFuncPtr(_glStencilOpSeparate, "glStencilOpSeparate");
    setGLExtensionFuncPtr(_glStencilFuncSeparate, "glStencilFuncSeparate");
    setGLExtensionFuncPtr(_glStencilMaskSeparate, "glStencilMaskSeparate");
    setGLExtensionFuncPtr(_glAttachShader, "glAttachShader", "glAttachObjectARB");
    setGLExtensionFuncPtr(_glBindAttribLocation, "glBindAttribLocation", "glBindAttribLocationARB");
    setGLExtensionFuncPtr(_glCompileShader, "glCompileShader", "glCompileShaderARB");
    setGLExtensionFuncPtr(_glCreateProgram, "glCreateProgram", "glCreateProgramObjectARB");
    setGLExtensionFuncPtr(_glCreateShader, "glCreateShader", "glCreateShaderObjectARB");
    setGLExtensionFuncPtr(_glDeleteProgram, "glDeleteProgram");
    setGLExtensionFuncPtr(_glDeleteShader, "glDeleteShader");
    setGLExtensionFuncPtr(_glDetachShader, "glDetachShader", "glDetachObjectARB");
    setGLExtensionFuncPtr(_glDisableVertexAttribArray, "glDisableVertexAttribArray");
    setGLExtensionFuncPtr(_glEnableVertexAttribArray, "glEnableVertexAttribArray");
    setGLExtensionFuncPtr(_glGetActiveAttrib, "glGetActiveAttrib", "glGetActiveAttribARB");
    setGLExtensionFuncPtr(_glGetActiveUniform, "glGetActiveUniform", "glGetActiveUniformARB");
    setGLExtensionFuncPtr(_glGetAttachedShaders, "glGetAttachedShaders", "glGetAttachedObjectsARB");
    setGLExtensionFuncPtr(_glGetAttribLocation, "glGetAttribLocation", "glGetAttribLocationARB");
    setGLExtensionFuncPtr(_glGetProgramiv, "glGetProgramiv");
    setGLExtensionFuncPtr(_glGetProgramInfoLog, "glGetProgramInfoLog");
    setGLExtensionFuncPtr(_glGetShaderiv, "glGetShaderiv");
    setGLExtensionFuncPtr(_glGetShaderInfoLog, "glGetShaderInfoLog");
    setGLExtensionFuncPtr(_glGetShaderSource, "glGetShaderSource", "glGetShaderSourceARB");
    setGLExtensionFuncPtr(_glGetUniformLocation, "glGetUniformLocation", "glGetUniformLocationARB");
    setGLExtensionFuncPtr(_glGetUniformfv, "glGetUniformfv", "glGetUniformfvARB");
    setGLExtensionFuncPtr(_glGetUniformiv, "glGetUniformiv", "glGetUniformivARB");
    setGLExtensionFuncPtr(_glGetVertexAttribdv, "glGetVertexAttribdv");
    setGLExtensionFuncPtr(_glGetVertexAttribfv, "glGetVertexAttribfv");
    setGLExtensionFuncPtr(_glGetVertexAttribiv, "glGetVertexAttribiv");
    setGLExtensionFuncPtr(_glGetVertexAttribPointerv, "glGetVertexAttribPointerv");
    setGLExtensionFuncPtr(_glIsProgram, "glIsProgram");
    setGLExtensionFuncPtr(_glIsShader, "glIsShader");
    setGLExtensionFuncPtr(_glLinkProgram, "glLinkProgram", "glLinkProgramARB");
    setGLExtensionFuncPtr(_glShaderSource, "glShaderSource", "glShaderSourceARB");
    setGLExtensionFuncPtr(_glUseProgram, "glUseProgram", "glUseProgramObjectARB");
    setGLExtensionFuncPtr(_glUniform1f, "glUniform1f", "glUniform1fARB");
    setGLExtensionFuncPtr(_glUniform2f, "glUniform2f", "glUniform2fARB");
    setGLExtensionFuncPtr(_glUniform3f, "glUniform3f", "glUniform3fARB");
    setGLExtensionFuncPtr(_glUniform4f, "glUniform4f", "glUniform4fARB");
    setGLExtensionFuncPtr(_glUniform1i, "glUniform1i", "glUniform1iARB");
    setGLExtensionFuncPtr(_glUniform2i, "glUniform2i", "glUniform2iARB");
    setGLExtensionFuncPtr(_glUniform3i, "glUniform3i", "glUniform3iARB");
    setGLExtensionFuncPtr(_glUniform4i, "glUniform4i", "glUniform4iARB");
    setGLExtensionFuncPtr(_glUniform1fv, "glUniform1fv", "glUniform1fvARB");
    setGLExtensionFuncPtr(_glUniform2fv, "glUniform2fv", "glUniform2fvARB");
    setGLExtensionFuncPtr(_glUniform3fv, "glUniform3fv", "glUniform3fvARB");
    setGLExtensionFuncPtr(_glUniform4fv, "glUniform4fv", "glUniform4fvARB");
    setGLExtensionFuncPtr(_glUniform1iv, "glUniform1iv", "glUniform1ivARB");
    setGLExtensionFuncPtr(_glUniform2iv, "glUniform2iv", "glUniform2ivARB");
    setGLExtensionFuncPtr(_glUniform3iv, "glUniform3iv", "glUniform3ivARB");
    setGLExtensionFuncPtr(_glUniform4iv, "glUniform4iv", "glUniform4ivARB");
    setGLExtensionFuncPtr(_glUniformMatrix2fv, "glUniformMatrix2fv", "glUniformMatrix2fvARB");
    setGLExtensionFuncPtr(_glUniformMatrix3fv, "glUniformMatrix3fv", "glUniformMatrix3fvARB");
    setGLExtensionFuncPtr(_glUniformMatrix4fv, "glUniformMatrix4fv", "glUniformMatrix4fvARB");
    setGLExtensionFuncPtr(_glValidateProgram, "glValidateProgram", "glValidateProgramARB");
    setGLExtensionFuncPtr(_glVertexAttrib1d, "glVertexAttrib1d");
    setGLExtensionFuncPtr(_glVertexAttrib1dv, "glVertexAttrib1dv");
    setGLExtensionFuncPtr(_glVertexAttrib1f, "glVertexAttrib1f");
    setGLExtensionFuncPtr(_glVertexAttrib1fv, "glVertexAttrib1fv");
    setGLExtensionFuncPtr(_glVertexAttrib1s, "glVertexAttrib1s");
    setGLExtensionFuncPtr(_glVertexAttrib1sv, "glVertexAttrib1sv");
    setGLExtensionFuncPtr(_glVertexAttrib2d, "glVertexAttrib2d");
    setGLExtensionFuncPtr(_glVertexAttrib2dv, "glVertexAttrib2dv");
    setGLExtensionFuncPtr(_glVertexAttrib2f, "glVertexAttrib2f");
    setGLExtensionFuncPtr(_glVertexAttrib2fv, "glVertexAttrib2fv");
    setGLExtensionFuncPtr(_glVertexAttrib2s, "glVertexAttrib2s");
    setGLExtensionFuncPtr(_glVertexAttrib2sv, "glVertexAttrib2sv");
    setGLExtensionFuncPtr(_glVertexAttrib3d, "glVertexAttrib3d");
    setGLExtensionFuncPtr(_glVertexAttrib3dv, "glVertexAttrib3dv");
    setGLExtensionFuncPtr(_glVertexAttrib3f, "glVertexAttrib3f");
    setGLExtensionFuncPtr(_glVertexAttrib3fv, "glVertexAttrib3fv");
    setGLExtensionFuncPtr(_glVertexAttrib3s, "glVertexAttrib3s");
    setGLExtensionFuncPtr(_glVertexAttrib3sv, "glVertexAttrib3sv");
    setGLExtensionFuncPtr(_glVertexAttrib4Nbv, "glVertexAttrib4Nbv");
    setGLExtensionFuncPtr(_glVertexAttrib4Niv, "glVertexAttrib4Niv");
    setGLExtensionFuncPtr(_glVertexAttrib4Nsv, "glVertexAttrib4Nsv");
    setGLExtensionFuncPtr(_glVertexAttrib4Nub, "glVertexAttrib4Nub");
    setGLExtensionFuncPtr(_glVertexAttrib4Nubv, "glVertexAttrib4Nubv");
    setGLExtensionFuncPtr(_glVertexAttrib4Nuiv, "glVertexAttrib4Nuiv");
    setGLExtensionFuncPtr(_glVertexAttrib4Nusv, "glVertexAttrib4Nusv");
    setGLExtensionFuncPtr(_glVertexAttrib4bv, "glVertexAttrib4bv");
    setGLExtensionFuncPtr(_glVertexAttrib4d, "glVertexAttrib4d");
    setGLExtensionFuncPtr(_glVertexAttrib4dv, "glVertexAttrib4dv");
    setGLExtensionFuncPtr(_glVertexAttrib4f, "glVertexAttrib4f");
    setGLExtensionFuncPtr(_glVertexAttrib4fv, "glVertexAttrib4fv");
    setGLExtensionFuncPtr(_glVertexAttrib4iv, "glVertexAttrib4iv");
    setGLExtensionFuncPtr(_glVertexAttrib4s, "glVertexAttrib4s");
    setGLExtensionFuncPtr(_glVertexAttrib4sv, "glVertexAttrib4sv");
    setGLExtensionFuncPtr(_glVertexAttrib4ubv, "glVertexAttrib4ubv");
    setGLExtensionFuncPtr(_glVertexAttrib4uiv, "glVertexAttrib4uiv");
    setGLExtensionFuncPtr(_glVertexAttrib4usv, "glVertexAttrib4usv");
    setGLExtensionFuncPtr(_glVertexAttribPointer, "glVertexAttribPointer");
    setGLExtensionFuncPtr(_glVertexAttribDivisor, "glVertexAttribDivisor");

    // v1.5-only ARB entry points, in case they're needed for fallback
    setGLExtensionFuncPtr(_glGetInfoLogARB, "glGetInfoLogARB");
    setGLExtensionFuncPtr(_glGetObjectParameterivARB, "glGetObjectParameterivARB");
    setGLExtensionFuncPtr(_glDeleteObjectARB, "glDeleteObjectARB");
    setGLExtensionFuncPtr(_glGetHandleARB, "glGetHandleARB");

    // GL 2.1
    setGLExtensionFuncPtr(_glUniformMatrix2x3fv,  "glUniformMatrix2x3fv" );
    setGLExtensionFuncPtr(_glUniformMatrix3x2fv,  "glUniformMatrix3x2fv" );
    setGLExtensionFuncPtr(_glUniformMatrix2x4fv,  "glUniformMatrix2x4fv" );
    setGLExtensionFuncPtr(_glUniformMatrix4x2fv,  "glUniformMatrix4x2fv" );
    setGLExtensionFuncPtr(_glUniformMatrix3x4fv,  "glUniformMatrix3x4fv" );
    setGLExtensionFuncPtr(_glUniformMatrix4x3fv,  "glUniformMatrix4x3fv" );

    // EXT_geometry_shader4
    setGLExtensionFuncPtr(_glProgramParameteri,  "glProgramParameteri", "glProgramParameteriEXT" );
    setGLExtensionFuncPtr(_glFramebufferTexture,  "glFramebufferTexture", "glFramebufferTextureEXT" );
    setGLExtensionFuncPtr(_glFramebufferTextureLayer,  "glFramebufferTextureLayer", "glFramebufferTextureLayerEXT" );
    setGLExtensionFuncPtr(_glFramebufferTextureFace,  "glFramebufferTextureFace", "glFramebufferTextureFaceEXT" );

    // ARB_tesselation_shader
    setGLExtensionFuncPtr(_glPatchParameteri, "glPatchParameteri" );
    setGLExtensionFuncPtr(_glPatchParameterfv, "glPatchParameterfv");

    // EXT_gpu_shader4
    setGLExtensionFuncPtr(_glGetUniformuiv,  "glGetUniformuiv", "glGetUniformuivEXT" );
    setGLExtensionFuncPtr(_glBindFragDataLocation,  "glBindFragDataLocation", "glBindFragDataLocationEXT" );
    setGLExtensionFuncPtr(_glGetFragDataLocation,  "glGetFragDataLocation", "glGetFragDataLocationEXT" );
    setGLExtensionFuncPtr(_glUniform1ui,  "glUniform1ui", "glUniform1uiEXT" );
    setGLExtensionFuncPtr(_glUniform2ui,  "glUniform2ui", "glUniform2uiEXT" );
    setGLExtensionFuncPtr(_glUniform3ui,  "glUniform3ui", "glUniform3uiEXT" );
    setGLExtensionFuncPtr(_glUniform4ui,  "glUniform4ui", "glUniform4uiEXT" );
    setGLExtensionFuncPtr(_glUniform1uiv,  "glUniform1uiv", "glUniform1uivEXT" );
    setGLExtensionFuncPtr(_glUniform2uiv,  "glUniform2uiv", "glUniform2uivEXT" );
    setGLExtensionFuncPtr(_glUniform3uiv,  "glUniform3uiv", "glUniform3uivEXT" );
    setGLExtensionFuncPtr(_glUniform4uiv,  "glUniform4uiv", "glUniform4uivEXT" );
    // ARB_uniform_buffer_object
    setGLExtensionFuncPtr(_glGetUniformIndices, "glGetUniformIndices");
    setGLExtensionFuncPtr(_glGetActiveUniformsiv, "glGetActiveUniformsiv");
    setGLExtensionFuncPtr(_glGetActiveUniformName, "glGetActiveUniformName");
    setGLExtensionFuncPtr(_glGetUniformBlockIndex, "glGetUniformBlockIndex");
    setGLExtensionFuncPtr(_glGetActiveUniformBlockiv, "glGetActiveUniformBlockiv");
    setGLExtensionFuncPtr(_glGetActiveUniformBlockName, "glGetActiveUniformBlockName");
    setGLExtensionFuncPtr(_glUniformBlockBinding, "glUniformBlockBinding");

    // ARB_get_program_binary
    setGLExtensionFuncPtr(_glGetProgramBinary, "glGetProgramBinary");
    setGLExtensionFuncPtr(_glProgramBinary, "glProgramBinary");

    // ARB_gpu_shader_fp64
    setGLExtensionFuncPtr(_glUniform1d, "glUniform1d" );
    setGLExtensionFuncPtr(_glUniform2d, "glUniform2d" );
    setGLExtensionFuncPtr(_glUniform3d, "glUniform3d" );
    setGLExtensionFuncPtr(_glUniform4d, "glUniform4d" );
    setGLExtensionFuncPtr(_glUniform1dv, "glUniform1dv" );
    setGLExtensionFuncPtr(_glUniform2dv, "glUniform2dv" );
    setGLExtensionFuncPtr(_glUniform3dv, "glUniform3dv" );
    setGLExtensionFuncPtr(_glUniform4dv, "glUniform4dv" );
    setGLExtensionFuncPtr(_glUniformMatrix2dv, "glUniformMatrix2dv" );
    setGLExtensionFuncPtr(_glUniformMatrix3dv, "glUniformMatrix3dv" );
    setGLExtensionFuncPtr(_glUniformMatrix4dv, "glUniformMatrix4dv" );
    setGLExtensionFuncPtr(_glUniformMatrix2x3dv,  "glUniformMatrix2x3dv" );
    setGLExtensionFuncPtr(_glUniformMatrix3x2dv,  "glUniformMatrix3x2dv" );
    setGLExtensionFuncPtr(_glUniformMatrix2x4dv,  "glUniformMatrix2x4dv" );
    setGLExtensionFuncPtr(_glUniformMatrix4x2dv,  "glUniformMatrix4x2dv" );
    setGLExtensionFuncPtr(_glUniformMatrix3x4dv,  "glUniformMatrix3x4dv" );
    setGLExtensionFuncPtr(_glUniformMatrix4x3dv,  "glUniformMatrix4x3dv" );

    // ARB_shader_atomic_counters
    setGLExtensionFuncPtr(_glGetActiveAtomicCounterBufferiv,  "glGetActiveAtomicCounterBufferiv" );

    // ARB_compute_shader
    setGLExtensionFuncPtr(_glDispatchCompute,  "glDispatchCompute" );
}


bool GL2Extensions::isGlslSupported() const
{
    return ( _glVersion >= 2.0f ) ||
           ( _isShaderObjectsSupported &&
             _isVertexShaderSupported &&
             _isFragmentShaderSupported &&
             _isLanguage100Supported );
}


///////////////////////////////////////////////////////////////////////////
// Static array of per-context osg::GL2Extensions instances

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

static void NotSupported( const char* funcName )
{
    OSG_WARN
        <<"Error: "<<funcName<<" not supported by OpenGL driver"<<std::endl;
}



void GL2Extensions::glBlendEquationSeparate(GLenum modeRGB, GLenum modeAlpha) const
{
    if (_glBlendEquationSeparate)
    {
        _glBlendEquationSeparate(modeRGB, modeAlpha);
    }
    else
    {
        NotSupported( "glBlendEquationSeparate" );
    }
}


void GL2Extensions::glDrawBuffers(GLsizei n, const GLenum *bufs) const
{
    if (_glDrawBuffers)
    {
        _glDrawBuffers(n, bufs);
    }
    else
    {
        NotSupported( "glDrawBuffers" );
    }
}


void GL2Extensions::glStencilOpSeparate(GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass) const
{
    if (_glStencilOpSeparate)
    {
        _glStencilOpSeparate(face, sfail, dpfail, dppass);
    }
    else
    {
        NotSupported( "glStencilOpSeparate" );
    }
}


void GL2Extensions::glStencilFuncSeparate(GLenum frontfunc, GLenum backfunc, GLint ref, GLuint mask) const
{
    if (_glStencilFuncSeparate)
    {
        _glStencilFuncSeparate(frontfunc, backfunc, ref, mask);
    }
    else
    {
        NotSupported( "glStencilFuncSeparate" );
    }
}


void GL2Extensions::glStencilMaskSeparate(GLenum face, GLuint mask) const
{
    if (_glStencilMaskSeparate)
    {
        _glStencilMaskSeparate(face, mask);
    }
    else
    {
        NotSupported( "glStencilMaskSeparate" );
    }
}


void GL2Extensions::glAttachShader(GLuint program, GLuint shader) const
{
    if (_glAttachShader)
    {
        _glAttachShader(program, shader);
    }
    else
    {
        NotSupported( "glAttachShader" );
    }
}


void GL2Extensions::glBindAttribLocation(GLuint program, GLuint index, const GLchar *name) const
{
    if (_glBindAttribLocation)
    {
        _glBindAttribLocation(program, index, name);
    }
    else
    {
        NotSupported( "glBindAttribLocation" );
    }
}


void GL2Extensions::glCompileShader(GLuint shader) const
{
    if (_glCompileShader)
    {
        _glCompileShader(shader);
    }
    else
    {
        NotSupported( "glCompileShader" );
    }
}


GLuint GL2Extensions::glCreateProgram(void) const
{
    if (_glCreateProgram)
    {
        return _glCreateProgram();
    }
    else
    {
        NotSupported( "glCreateProgram" );
        return 0;
    }
}


GLuint GL2Extensions::glCreateShader(GLenum type) const
{
    if (_glCreateShader)
    {
        return _glCreateShader(type);
    }
    else
    {
        NotSupported( "glCreateShader" );
        return 0;
    }
}


void GL2Extensions::glDeleteProgram(GLuint program) const
{
    if (_glDeleteProgram)
    {
        _glDeleteProgram(program);
    }
    else if (_glDeleteObjectARB)
    {
        _glDeleteObjectARB(program);
    }
    else
    {
        NotSupported( "glDeleteProgram" );
    }
}


void GL2Extensions::glDeleteShader(GLuint shader) const
{
    if (_glDeleteShader)
    {
        _glDeleteShader(shader);
    }
    else if (_glDeleteObjectARB)
    {
        _glDeleteObjectARB(shader);
    }
    else
    {
        NotSupported( "glDeleteShader" );
    }
}


void GL2Extensions::glDetachShader(GLuint program, GLuint shader) const
{
    if (_glDetachShader)
    {
        _glDetachShader(program, shader);
    }
    else
    {
        NotSupported( "glDetachShader" );
    }
}


void GL2Extensions::glDisableVertexAttribArray(GLuint index) const
{
    if (_glDisableVertexAttribArray)
    {
        _glDisableVertexAttribArray(index);
    }
    else
    {
        NotSupported( "glDisableVertexAttribArray" );
    }
}


void GL2Extensions::glEnableVertexAttribArray(GLuint index) const
{
    if (_glEnableVertexAttribArray)
    {
        _glEnableVertexAttribArray(index);
    }
    else
    {
        NotSupported( "glEnableVertexAttribArray" );
    }
}


void GL2Extensions::glGetActiveAttrib(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name) const
{
    if (_glGetActiveAttrib)
    {
        _glGetActiveAttrib(program, index, bufSize, length, size, type, name);
    }
    else
    {
        NotSupported( "glGetActiveAttrib" );
    }
}


void GL2Extensions::glGetActiveUniform(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name) const
{
    if (_glGetActiveUniform)
    {
        _glGetActiveUniform(program, index, bufSize, length, size, type, name);
    }
    else
    {
        NotSupported( "glGetActiveUniform" );
    }
}


void GL2Extensions::glGetAttachedShaders(GLuint program, GLsizei maxCount, GLsizei *count, GLuint *obj) const
{
    if (_glGetAttachedShaders)
    {
        _glGetAttachedShaders(program, maxCount, count, obj);
    }
    else
    {
        NotSupported( "glGetAttachedShaders" );
    }
}


GLint GL2Extensions::glGetAttribLocation(GLuint program, const GLchar *name) const
{
    if (_glGetAttribLocation)
    {
        return _glGetAttribLocation(program, name);
    }
    else
    {
        NotSupported( "glGetAttribLocation" );
        return 0;
    }
}


void GL2Extensions::glGetProgramiv(GLuint program, GLenum pname, GLint *params) const
{
    if (_glGetProgramiv)
    {

        _glGetProgramiv(program, pname, params);
    }
    else if (_glGetObjectParameterivARB)
    {

        _glGetObjectParameterivARB(program, pname, params);
    }
    else
    {
        NotSupported( "glGetProgramiv" );
    }
}


void GL2Extensions::glGetProgramInfoLog(GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog) const
{
    if (_glGetProgramInfoLog)
    {

        _glGetProgramInfoLog(program, bufSize, length, infoLog);
    }
    else if (_glGetInfoLogARB)
    {

        _glGetInfoLogARB(program, bufSize, length, infoLog);
    }
    else
    {
        NotSupported( "glGetProgramInfoLog" );
    }
}


void GL2Extensions::glGetShaderiv(GLuint shader, GLenum pname, GLint *params) const
{
    if (_glGetShaderiv)
    {

        _glGetShaderiv(shader, pname, params);
    }
    else if (_glGetObjectParameterivARB)
    {

        _glGetObjectParameterivARB(shader, pname, params);
    }
    else
    {
        NotSupported( "glGetShaderiv" );
    }
}


void GL2Extensions::glGetShaderInfoLog(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog) const
{
    if (_glGetShaderInfoLog)
    {

        _glGetShaderInfoLog(shader, bufSize, length, infoLog);
    }
    else if (_glGetInfoLogARB)
    {

        _glGetInfoLogARB(shader, bufSize, length, infoLog);
    }
    else
    {
        NotSupported( "glGetShaderInfoLog" );
    }
}


void GL2Extensions::glGetShaderSource(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source) const
{
    if (_glGetShaderSource)
    {

        _glGetShaderSource(shader, bufSize, length, source);
    }
    else
    {
        NotSupported( "glGetShaderSource" );
    }
}


GLint GL2Extensions::glGetUniformLocation(GLuint program, const GLchar *name) const
{
    if (_glGetUniformLocation)
    {

        return _glGetUniformLocation(program, name);
    }
    else
    {
        NotSupported( "glGetUniformLocation" );
        return 0;
    }
}


void GL2Extensions::glGetUniformfv(GLuint program, GLint location, GLfloat *params) const
{
    if (_glGetUniformfv)
    {

        _glGetUniformfv(program, location, params);
    }
    else
    {
        NotSupported( "glGetUniformfv" );
    }
}


void GL2Extensions::glGetUniformiv(GLuint program, GLint location, GLint *params) const
{
    if (_glGetUniformiv)
    {

        _glGetUniformiv(program, location, params);
    }
    else
    {
        NotSupported( "glGetUniformiv" );
    }
}


void GL2Extensions::glGetVertexAttribdv(GLuint index, GLenum pname, GLdouble *params) const
{
    if (_glGetVertexAttribdv)
    {

        _glGetVertexAttribdv(index, pname, params);
    }
    else
    {
        NotSupported( "glGetVertexAttribdv" );
    }
}


void GL2Extensions::glGetVertexAttribfv(GLuint index, GLenum pname, GLfloat *params) const
{
    if (_glGetVertexAttribfv)
    {

        _glGetVertexAttribfv(index, pname, params);
    }
    else
    {
        NotSupported( "glGetVertexAttribfv" );
    }
}


void GL2Extensions::glGetVertexAttribiv(GLuint index, GLenum pname, GLint *params) const
{
    if (_glGetVertexAttribiv)
    {

        _glGetVertexAttribiv(index, pname, params);
    }
    else
    {
        NotSupported( "glGetVertexAttribiv" );
    }
}


void GL2Extensions::glGetVertexAttribPointerv(GLuint index, GLenum pname, GLvoid* *pointer) const
{
    if (_glGetVertexAttribPointerv)
    {

        _glGetVertexAttribPointerv(index, pname, pointer);
    }
    else
    {
        NotSupported( "glGetVertexAttribPointerv" );
    }
}


GLboolean GL2Extensions::glIsProgram(GLuint program) const
{
    if (_glIsProgram)
    {

        return _glIsProgram(program);
    }
    else
    {
        NotSupported( "glIsProgram" );
        return 0;
    }
}


GLboolean GL2Extensions::glIsShader(GLuint shader) const
{
    if (_glIsShader)
    {

        return _glIsShader(shader);
    }
    else
    {
        NotSupported( "glIsShader" );
        return 0;
    }
}


void GL2Extensions::glLinkProgram(GLuint program) const
{
    if (_glLinkProgram)
    {

        _glLinkProgram(program);
    }
    else
    {
        NotSupported( "glLinkProgram" );
    }
}


void GL2Extensions::glShaderSource(GLuint shader, GLsizei count, const GLchar* *string, const GLint *length) const
{
    if (_glShaderSource)
    {

        _glShaderSource(shader, count, string, length);
    }
    else
    {
        NotSupported( "glShaderSource" );
    }
}


void GL2Extensions::glUseProgram(GLuint program) const
{
    if (_glUseProgram)
    {

        _glUseProgram(program);
    }
    else
    {
        NotSupported( "glUseProgram" );
    }
}


void GL2Extensions::glUniform1f(GLint location, GLfloat v0) const
{
    if (_glUniform1f)
    {

        _glUniform1f(location, v0);
    }
    else
    {
        NotSupported( "glUniform1f" );
    }
}


void GL2Extensions::glUniform2f(GLint location, GLfloat v0, GLfloat v1) const
{
    if (_glUniform2f)
    {

        _glUniform2f(location, v0, v1);
    }
    else
    {
        NotSupported( "glUniform2f" );
    }
}


void GL2Extensions::glUniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2) const
{
    if (_glUniform3f)
    {

        _glUniform3f(location, v0, v1, v2);
    }
    else
    {
        NotSupported( "glUniform3f" );
    }
}


void GL2Extensions::glUniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) const
{
    if (_glUniform4f)
    {

        _glUniform4f(location, v0, v1, v2, v3);
    }
    else
    {
        NotSupported( "glUniform4f" );
    }
}


void GL2Extensions::glUniform1i(GLint location, GLint v0) const
{
    if (_glUniform1i)
    {

        _glUniform1i(location, v0);
    }
    else
    {
        NotSupported( "glUniform1i" );
    }
}


void GL2Extensions::glUniform2i(GLint location, GLint v0, GLint v1) const
{
    if (_glUniform2i)
    {

        _glUniform2i(location, v0, v1);
    }
    else
    {
        NotSupported( "glUniform2i" );
    }
}


void GL2Extensions::glUniform3i(GLint location, GLint v0, GLint v1, GLint v2) const
{
    if (_glUniform3i)
    {

        _glUniform3i(location, v0, v1, v2);
    }
    else
    {
        NotSupported( "glUniform3i" );
    }
}


void GL2Extensions::glUniform4i(GLint location, GLint v0, GLint v1, GLint v2, GLint v3) const
{
    if (_glUniform4i)
    {

        _glUniform4i(location, v0, v1, v2, v3);
    }
    else
    {
        NotSupported( "glUniform4i" );
    }
}


void GL2Extensions::glUniform1fv(GLint location, GLsizei count, const GLfloat *value) const
{
    if (_glUniform1fv)
    {

        _glUniform1fv(location, count, value);
    }
    else
    {
        NotSupported( "glUniform1fv" );
    }
}


void GL2Extensions::glUniform2fv(GLint location, GLsizei count, const GLfloat *value) const
{
    if (_glUniform2fv)
    {

        _glUniform2fv(location, count, value);
    }
    else
    {
        NotSupported( "glUniform2fv" );
    }
}


void GL2Extensions::glUniform3fv(GLint location, GLsizei count, const GLfloat *value) const
{
    if (_glUniform3fv)
    {

        _glUniform3fv(location, count, value);
    }
    else
    {
        NotSupported( "glUniform3fv" );
    }
}


void GL2Extensions::glUniform4fv(GLint location, GLsizei count, const GLfloat *value) const
{
    if (_glUniform4fv)
    {

        _glUniform4fv(location, count, value);
    }
    else
    {
        NotSupported( "glUniform4fv" );
    }
}


void GL2Extensions::glUniform1iv(GLint location, GLsizei count, const GLint *value) const
{
    if (_glUniform1iv)
    {

        _glUniform1iv(location, count, value);
    }
    else
    {
        NotSupported( "glUniform1iv" );
    }
}


void GL2Extensions::glUniform2iv(GLint location, GLsizei count, const GLint *value) const
{
    if (_glUniform2iv)
    {

        _glUniform2iv(location, count, value);
    }
    else
    {
        NotSupported( "glUniform2iv" );
    }
}


void GL2Extensions::glUniform3iv(GLint location, GLsizei count, const GLint *value) const
{
    if (_glUniform3iv)
    {

        _glUniform3iv(location, count, value);
    }
    else
    {
        NotSupported( "glUniform3iv" );
    }
}


void GL2Extensions::glUniform4iv(GLint location, GLsizei count, const GLint *value) const
{
    if (_glUniform4iv)
    {

        _glUniform4iv(location, count, value);
    }
    else
    {
        NotSupported( "glUniform4iv" );
    }
}


void GL2Extensions::glUniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) const
{
    if (_glUniformMatrix2fv)
    {

        _glUniformMatrix2fv(location, count, transpose, value);
    }
    else
    {
        NotSupported( "glUniformMatrix2fv" );
    }
}


void GL2Extensions::glUniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) const
{
    if (_glUniformMatrix3fv)
    {

        _glUniformMatrix3fv(location, count, transpose, value);
    }
    else
    {
        NotSupported( "glUniformMatrix3fv" );
    }
}


void GL2Extensions::glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) const
{
    if (_glUniformMatrix4fv)
    {

        _glUniformMatrix4fv(location, count, transpose, value);
    }
    else
    {
        NotSupported( "glUniformMatrix4fv" );
    }
}


void GL2Extensions::glValidateProgram(GLuint program) const
{
    if (_glValidateProgram)
    {

        _glValidateProgram(program);
    }
    else
    {
        NotSupported( "glValidateProgram" );
    }
}


void GL2Extensions::glVertexAttrib1d(GLuint index, GLdouble x) const
{
    if (_glVertexAttrib1d)
    {

        _glVertexAttrib1d(index, x);
    }
    else
    {
        NotSupported( "glVertexAttrib1d" );
    }
}


void GL2Extensions::glVertexAttrib1dv(GLuint index, const GLdouble *v) const
{
    if (_glVertexAttrib1dv)
    {

        _glVertexAttrib1dv(index, v);
    }
    else
    {
        NotSupported( "glVertexAttrib1dv" );
    }
}


void GL2Extensions::glVertexAttrib1f(GLuint index, GLfloat x) const
{
    if (_glVertexAttrib1f)
    {

        _glVertexAttrib1f(index, x);
    }
    else
    {
        NotSupported( "glVertexAttrib1f" );
    }
}


void GL2Extensions::glVertexAttrib1fv(GLuint index, const GLfloat *v) const
{
    if (_glVertexAttrib1fv)
    {

        _glVertexAttrib1fv(index, v);
    }
    else
    {
        NotSupported( "glVertexAttrib1fv" );
    }
}


void GL2Extensions::glVertexAttrib1s(GLuint index, GLshort x) const
{
    if (_glVertexAttrib1s)
    {

        _glVertexAttrib1s(index, x);
    }
    else
    {
        NotSupported( "glVertexAttrib1s" );
    }
}


void GL2Extensions::glVertexAttrib1sv(GLuint index, const GLshort *v) const
{
    if (_glVertexAttrib1sv)
    {

        _glVertexAttrib1sv(index, v);
    }
    else
    {
        NotSupported( "glVertexAttrib1sv" );
    }
}


void GL2Extensions::glVertexAttrib2d(GLuint index, GLdouble x, GLdouble y) const
{
    if (_glVertexAttrib2d)
    {

        _glVertexAttrib2d(index, x, y);
    }
    else
    {
        NotSupported( "glVertexAttrib2d" );
    }
}


void GL2Extensions::glVertexAttrib2dv(GLuint index, const GLdouble *v) const
{
    if (_glVertexAttrib2dv)
    {

        _glVertexAttrib2dv(index, v);
    }
    else
    {
        NotSupported( "glVertexAttrib2dv" );
    }
}


void GL2Extensions::glVertexAttrib2f(GLuint index, GLfloat x, GLfloat y) const
{
    if (_glVertexAttrib2f)
    {

        _glVertexAttrib2f(index, x, y);
    }
    else
    {
        NotSupported( "glVertexAttrib2f" );
    }
}


void GL2Extensions::glVertexAttrib2fv(GLuint index, const GLfloat *v) const
{
    if (_glVertexAttrib2fv)
    {

        _glVertexAttrib2fv(index, v);
    }
    else
    {
        NotSupported( "glVertexAttrib2fv" );
    }
}


void GL2Extensions::glVertexAttrib2s(GLuint index, GLshort x, GLshort y) const
{
    if (_glVertexAttrib2s)
    {

        _glVertexAttrib2s(index, x, y);
    }
    else
    {
        NotSupported( "glVertexAttrib2s" );
    }
}


void GL2Extensions::glVertexAttrib2sv(GLuint index, const GLshort *v) const
{
    if (_glVertexAttrib2sv)
    {

        _glVertexAttrib2sv(index, v);
    }
    else
    {
        NotSupported( "glVertexAttrib2sv" );
    }
}


void GL2Extensions::glVertexAttrib3d(GLuint index, GLdouble x, GLdouble y, GLdouble z) const
{
    if (_glVertexAttrib3d)
    {

        _glVertexAttrib3d(index, x, y, z);
    }
    else
    {
        NotSupported( "glVertexAttrib3d" );
    }
}


void GL2Extensions::glVertexAttrib3dv(GLuint index, const GLdouble *v) const
{
    if (_glVertexAttrib3dv)
    {

        _glVertexAttrib3dv(index, v);
    }
    else
    {
        NotSupported( "glVertexAttrib3dv" );
    }
}


void GL2Extensions::glVertexAttrib3f(GLuint index, GLfloat x, GLfloat y, GLfloat z) const
{
    if (_glVertexAttrib3f)
    {

        _glVertexAttrib3f(index, x, y, z);
    }
    else
    {
        NotSupported( "glVertexAttrib3f" );
    }
}


void GL2Extensions::glVertexAttrib3fv(GLuint index, const GLfloat *v) const
{
    if (_glVertexAttrib3fv)
    {

        _glVertexAttrib3fv(index, v);
    }
    else
    {
        NotSupported( "glVertexAttrib3fv" );
    }
}


void GL2Extensions::glVertexAttrib3s(GLuint index, GLshort x, GLshort y, GLshort z) const
{
    if (_glVertexAttrib3s)
    {

        _glVertexAttrib3s(index, x, y, z);
    }
    else
    {
        NotSupported( "glVertexAttrib3s" );
    }
}


void GL2Extensions::glVertexAttrib3sv(GLuint index, const GLshort *v) const
{
    if (_glVertexAttrib3sv)
    {

        _glVertexAttrib3sv(index, v);
    }
    else
    {
        NotSupported( "glVertexAttrib3sv" );
    }
}


void GL2Extensions::glVertexAttrib4Nbv(GLuint index, const GLbyte *v) const
{
    if (_glVertexAttrib4Nbv)
    {

        _glVertexAttrib4Nbv(index, v);
    }
    else
    {
        NotSupported( "glVertexAttrib4Nbv" );
    }
}


void GL2Extensions::glVertexAttrib4Niv(GLuint index, const GLint *v) const
{
    if (_glVertexAttrib4Niv)
    {

        _glVertexAttrib4Niv(index, v);
    }
    else
    {
        NotSupported( "glVertexAttrib4Niv" );
    }
}


void GL2Extensions::glVertexAttrib4Nsv(GLuint index, const GLshort *v) const
{
    if (_glVertexAttrib4Nsv)
    {

        _glVertexAttrib4Nsv(index, v);
    }
    else
    {
        NotSupported( "glVertexAttrib4Nsv" );
    }
}


void GL2Extensions::glVertexAttrib4Nub(GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w) const
{
    if (_glVertexAttrib4Nub)
    {

        _glVertexAttrib4Nub(index, x, y, z, w);
    }
    else
    {
        NotSupported( "glVertexAttrib4Nub" );
    }
}


void GL2Extensions::glVertexAttrib4Nubv(GLuint index, const GLubyte *v) const
{
    if (_glVertexAttrib4Nubv)
    {

        _glVertexAttrib4Nubv(index, v);
    }
    else
    {
        NotSupported( "glVertexAttrib4Nubv" );
    }
}


void GL2Extensions::glVertexAttrib4Nuiv(GLuint index, const GLuint *v) const
{
    if (_glVertexAttrib4Nuiv)
    {

        _glVertexAttrib4Nuiv(index, v);
    }
    else
    {
        NotSupported( "glVertexAttrib4Nuiv" );
    }
}


void GL2Extensions::glVertexAttrib4Nusv(GLuint index, const GLushort *v) const
{
    if (_glVertexAttrib4Nusv)
    {

        _glVertexAttrib4Nusv(index, v);
    }
    else
    {
        NotSupported( "glVertexAttrib4Nusv" );
    }
}


void GL2Extensions::glVertexAttrib4bv(GLuint index, const GLbyte *v) const
{
    if (_glVertexAttrib4bv)
    {

        _glVertexAttrib4bv(index, v);
    }
    else
    {
        NotSupported( "glVertexAttrib4bv" );
    }
}


void GL2Extensions::glVertexAttrib4d(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w) const
{
    if (_glVertexAttrib4d)
    {

        _glVertexAttrib4d(index, x, y, z, w);
    }
    else
    {
        NotSupported( "glVertexAttrib4d" );
    }
}


void GL2Extensions::glVertexAttrib4dv(GLuint index, const GLdouble *v) const
{
    if (_glVertexAttrib4dv)
    {

        _glVertexAttrib4dv(index, v);
    }
    else
    {
        NotSupported( "glVertexAttrib4dv" );
    }
}


void GL2Extensions::glVertexAttrib4f(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w) const
{
    if (_glVertexAttrib4f)
    {

        _glVertexAttrib4f(index, x, y, z, w);
    }
    else
    {
        NotSupported( "glVertexAttrib4f" );
    }
}


void GL2Extensions::glVertexAttrib4fv(GLuint index, const GLfloat *v) const
{
    if (_glVertexAttrib4fv)
    {

        _glVertexAttrib4fv(index, v);
    }
    else
    {
        NotSupported( "glVertexAttrib4fv" );
    }
}


void GL2Extensions::glVertexAttrib4iv(GLuint index, const GLint *v) const
{
    if (_glVertexAttrib4iv)
    {

        _glVertexAttrib4iv(index, v);
    }
    else
    {
        NotSupported( "glVertexAttrib4iv" );
    }
}


void GL2Extensions::glVertexAttrib4s(GLuint index, GLshort x, GLshort y, GLshort z, GLshort w) const
{
    if (_glVertexAttrib4s)
    {

        _glVertexAttrib4s(index, x, y, z, w);
    }
    else
    {
        NotSupported( "glVertexAttrib4s" );
    }
}


void GL2Extensions::glVertexAttrib4sv(GLuint index, const GLshort *v) const
{
    if (_glVertexAttrib4sv)
    {

        _glVertexAttrib4sv(index, v);
    }
    else
    {
        NotSupported( "glVertexAttrib4sv" );
    }
}


void GL2Extensions::glVertexAttrib4ubv(GLuint index, const GLubyte *v) const
{
    if (_glVertexAttrib4ubv)
    {

        _glVertexAttrib4ubv(index, v);
    }
    else
    {
        NotSupported( "glVertexAttrib4ubv" );
    }
}


void GL2Extensions::glVertexAttrib4uiv(GLuint index, const GLuint *v) const
{
    if (_glVertexAttrib4uiv)
    {

        _glVertexAttrib4uiv(index, v);
    }
    else
    {
        NotSupported( "glVertexAttrib4uiv" );
    }
}


void GL2Extensions::glVertexAttrib4usv(GLuint index, const GLushort *v) const
{
    if (_glVertexAttrib4usv)
    {

        _glVertexAttrib4usv(index, v);
    }
    else
    {
        NotSupported( "glVertexAttrib4usv" );
    }
}


void GL2Extensions::glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer) const
{
    if (_glVertexAttribPointer)
    {

        _glVertexAttribPointer(index, size, type, normalized, stride, pointer);
    }
    else
    {
        NotSupported( "glVertexAttribPointer" );
    }
}

void GL2Extensions::glVertexAttribDivisor(GLuint index, GLuint divisor) const
{
    if (_glVertexAttribDivisor)
    {
        _glVertexAttribDivisor(index, divisor);
    }
    else
    {
        NotSupported( "glVertexAttribDivisor" );
    }
}

void GL2Extensions::glUniformMatrix2x3fv( GLint location, GLsizei count, GLboolean transpose, const GLfloat* value ) const
{
    if (_glUniformMatrix2x3fv)
    {

        _glUniformMatrix2x3fv( location, count, transpose, value );
    }
    else
    {
        NotSupported( "glUniformMatrix2x3fv" );
    }
}

void GL2Extensions::glUniformMatrix3x2fv( GLint location, GLsizei count, GLboolean transpose, const GLfloat* value ) const
{
    if (_glUniformMatrix3x2fv)
    {

        _glUniformMatrix3x2fv( location, count, transpose, value );
    }
    else
    {
        NotSupported( "glUniformMatrix3x2fv" );
    }
}

void GL2Extensions::glUniformMatrix2x4fv( GLint location, GLsizei count, GLboolean transpose, const GLfloat* value ) const
{
    if (_glUniformMatrix2x4fv)
    {

        _glUniformMatrix2x4fv( location, count, transpose, value );
    }
    else
    {
        NotSupported( "glUniformMatrix2x4fv" );
    }
}

void GL2Extensions::glUniformMatrix4x2fv( GLint location, GLsizei count, GLboolean transpose, const GLfloat* value ) const
{
    if (_glUniformMatrix4x2fv)
    {

        _glUniformMatrix4x2fv( location, count, transpose, value );
    }
    else
    {
        NotSupported( "glUniformMatrix4x2fv" );
    }
}

void GL2Extensions::glUniformMatrix3x4fv( GLint location, GLsizei count, GLboolean transpose, const GLfloat* value ) const
{
    if (_glUniformMatrix3x4fv)
    {

        _glUniformMatrix3x4fv( location, count, transpose, value );
    }
    else
    {
        NotSupported( "glUniformMatrix3x4fv" );
    }
}

void GL2Extensions::glUniformMatrix4x3fv( GLint location, GLsizei count, GLboolean transpose, const GLfloat* value ) const
{
    if (_glUniformMatrix4x3fv)
    {

        _glUniformMatrix4x3fv( location, count, transpose, value );
    }
    else
    {
        NotSupported( "glUniformMatrix4x3fv" );
    }
}


void GL2Extensions::glProgramParameteri( GLuint program, GLenum pname, GLint value ) const
{
    if (_glProgramParameteri)
    {

        _glProgramParameteri( program, pname, value );
    }
    else
    {
        NotSupported( "glProgramParameteri" );
    }
}

void GL2Extensions::glFramebufferTexture( GLenum target, GLenum attachment, GLuint texture, GLint level ) const
{
    if (_glFramebufferTexture)
    {

        _glFramebufferTexture( target, attachment, texture, level );
    }
    else
    {
        NotSupported( "glFramebufferTexture" );
    }
}

void GL2Extensions::glFramebufferTextureLayer( GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer ) const
{
    if (_glFramebufferTextureLayer)
    {

        _glFramebufferTextureLayer( target, attachment, texture, level, layer );
    }
    else
    {
        NotSupported( "glFramebufferTextureLayer" );
    }
}

void GL2Extensions::glFramebufferTextureFace( GLenum target, GLenum attachment, GLuint texture, GLint level, GLenum face ) const
{
    if (_glFramebufferTextureFace)
    {

        _glFramebufferTextureFace( target, attachment, texture, level, face );
    }
    else
    {
        NotSupported( "glFramebufferTextureFace" );
    }
}

void GL2Extensions::glPatchParameteri( GLenum pname, GLint value ) const
{
    if (_glPatchParameteri)
    {

        _glPatchParameteri( pname, value );
    }
    else
    {
        NotSupported( "glPatchParameteri" );
    }
}
void GL2Extensions::glPatchParameterfv( GLenum pname, const GLfloat* values ) const
{
    if (_glPatchParameterfv)
    {

        _glPatchParameterfv( pname, values );
    }
    else
    {
        NotSupported( "glPatchParameterfv" );
    }
}

void GL2Extensions::glGetUniformuiv( GLuint program, GLint location, GLuint* params ) const
{
    if (_glGetUniformuiv)
    {

        _glGetUniformuiv( program, location, params );
    }
    else
    {
        NotSupported( "glGetUniformuiv" );
    }
}

void GL2Extensions::glBindFragDataLocation( GLuint program, GLuint color, const GLchar* name ) const
{
    if (_glBindFragDataLocation)
    {

        _glBindFragDataLocation( program, color, name );
    }
    else
    {
        NotSupported( "glBindFragDataLocation" );
    }
}

GLint GL2Extensions::glGetFragDataLocation( GLuint program, const GLchar* name ) const
{
    if (_glGetFragDataLocation)
    {

        return _glGetFragDataLocation( program, name );
    }
    else
    {
        NotSupported( "glGetFragDataLocation" );
        return -1;
    }
}


void GL2Extensions::glUniform1ui( GLint location, GLuint v0 ) const
{
    if (_glUniform1ui)
    {

        _glUniform1ui( location, v0 );
    }
    else
    {
        NotSupported( "glUniform1ui" );
    }
}

void GL2Extensions::glUniform2ui( GLint location, GLuint v0, GLuint v1 ) const
{
    if (_glUniform2ui)
    {

        _glUniform2ui( location, v0, v1 );
    }
    else
    {
        NotSupported( "glUniform2ui" );
    }
}

void GL2Extensions::glUniform3ui( GLint location, GLuint v0, GLuint v1, GLuint v2 ) const
{
    if (_glUniform3ui)
    {

        _glUniform3ui( location, v0, v1, v2 );
    }
    else
    {
        NotSupported( "glUniform3ui" );
    }
}

void GL2Extensions::glUniform4ui( GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3 ) const
{
    if (_glUniform4ui)
    {

        _glUniform4ui( location, v0, v1, v2, v3 );
    }
    else
    {
        NotSupported( "glUniform4ui" );
    }
}

void GL2Extensions::glUniform1uiv( GLint location, GLsizei count, const GLuint *value ) const
{
    if (_glUniform1uiv)
    {

        _glUniform1uiv( location, count, value );
    }
    else
    {
        NotSupported( "glUniform1uiv" );
    }
}

void GL2Extensions::glUniform2uiv( GLint location, GLsizei count, const GLuint *value ) const
{
    if (_glUniform2uiv)
    {

        _glUniform2uiv( location, count, value );
    }
    else
    {
        NotSupported( "glUniform2uiv" );
    }
}

void GL2Extensions::glUniform3uiv( GLint location, GLsizei count, const GLuint *value ) const
{
    if (_glUniform3uiv)
    {

        _glUniform3uiv( location, count, value );
    }
    else
    {
        NotSupported( "glUniform3uiv" );
    }
}

void GL2Extensions::glUniform4uiv( GLint location, GLsizei count, const GLuint *value ) const
{
    if (_glUniform4uiv)
    {

        _glUniform4uiv( location, count, value );
    }
    else
    {
        NotSupported( "glUniform4uiv" );
    }
}

// ARB_uniform_buffer_object
void GL2Extensions::glGetUniformIndices(GLuint program, GLsizei uniformCount,
                                        const GLchar* *uniformNames,
                                        GLuint *uniformIndices) const
{
    if (_glGetUniformIndices)
    {
        _glGetUniformIndices(program, uniformCount, uniformNames,
                             uniformIndices);
    }
    else
    {
        NotSupported("glGetUniformIndices");
    }
}

void GL2Extensions::glGetActiveUniformsiv(GLuint program, GLsizei uniformCount,
                                          const GLuint *uniformIndices,
                                          GLenum pname, GLint *params) const
{
    if (_glGetActiveUniformsiv)
    {
        _glGetActiveUniformsiv(program, uniformCount, uniformIndices, pname,
                               params);
    }
    else
    {
        NotSupported("glGetActiveUniformsiv");
    }
}

void GL2Extensions::glGetActiveUniformName(GLuint program, GLuint uniformIndex,
                                           GLsizei bufSize, GLsizei *length,
                                           GLchar *uniformName) const
{
    if (_glGetActiveUniformName)
    {
        _glGetActiveUniformName(program, uniformIndex, bufSize, length,
                                uniformName);
    }
    else
    {
        NotSupported("glGetActiveUniformName");
    }
}

GLuint GL2Extensions::glGetUniformBlockIndex(GLuint program,
                                             const GLchar *uniformBlockName) const
{
    if (_glGetUniformBlockIndex)
    {
        return _glGetUniformBlockIndex(program, uniformBlockName);
    }
    else
    {
        NotSupported("glGetUniformBlockIndex");
        return GL_INVALID_INDEX;
    }
}

void GL2Extensions::glGetActiveUniformBlockiv(GLuint program,
                                              GLuint uniformBlockIndex,
                                              GLenum pname, GLint *params) const
{
    if (_glGetActiveUniformBlockiv)
    {
        _glGetActiveUniformBlockiv(program, uniformBlockIndex, pname, params);
    }
    else
    {
        NotSupported("glGetActiveUniformBlockiv");
    }
}

void GL2Extensions::glGetActiveUniformBlockName(GLuint program,
                                                GLuint uniformBlockIndex,
                                                GLsizei bufSize,
                                                GLsizei *length,
                                                GLchar *uniformBlockName) const
{
    if (_glGetActiveUniformBlockName)
    {
        _glGetActiveUniformBlockName(program, uniformBlockIndex, bufSize,
                                     length, uniformBlockName);
    }
    else
    {
        NotSupported("glGetActiveUniformBlockName");
    }
}

void GL2Extensions::glUniformBlockBinding(GLuint program,
                                          GLuint uniformBlockIndex,
                                          GLuint uniformBlockBinding) const
{
    if (_glUniformBlockBinding)
    {
        _glUniformBlockBinding(program, uniformBlockIndex, uniformBlockBinding);
    }
    else
    {
        NotSupported("glUniformBlockBinding");
    }
}

//ARB_get_program_binary
void GL2Extensions::glGetProgramBinary(GLuint program,
                                       GLsizei bufSize,
                                       GLsizei *length,
                                       GLenum *binaryFormat,
                                       GLvoid *binary) const
{
    if (_glGetProgramBinary)
    {
        _glGetProgramBinary(program, bufSize, length, binaryFormat, binary);
    }
    else
    {
        NotSupported("glGetProgramBinary");
    }
}

void GL2Extensions::glProgramBinary(GLuint program,
                                    GLenum binaryFormat,
                                    const GLvoid *binary,
                                    GLsizei length) const
{
    if (_glProgramBinary)
    {
        _glProgramBinary(program, binaryFormat, binary, length);
    }
    else
    {
        NotSupported("glProgramBinary");
    }
}

void GL2Extensions::glUniform1d(GLint location, GLdouble v0) const
{
    if (_glUniform1d)
    {

        _glUniform1d(location, v0);
    }
    else
    {
        NotSupported( "glUniform1d" );
    }
}

void GL2Extensions::glUniform2d(GLint location, GLdouble v0, GLdouble v1) const
{
    if (_glUniform2d)
    {

        _glUniform2d(location, v0, v1);
    }
    else
    {
        NotSupported( "glUniform2d" );
    }
}

void GL2Extensions::glUniform3d(GLint location, GLdouble v0, GLdouble v1, GLdouble v2) const
{
    if (_glUniform3d)
    {

        _glUniform3d(location, v0, v1, v2);
    }
    else
    {
        NotSupported( "glUniform3d" );
    }
}

void GL2Extensions::glUniform4d(GLint location, GLdouble v0, GLdouble v1, GLdouble v2, GLdouble v3) const
{
    if (_glUniform4d)
    {

        _glUniform4d(location, v0, v1, v2, v3);
    }
    else
    {
        NotSupported( "glUniform4d" );
    }
}

void GL2Extensions::glUniform1dv(GLint location, GLsizei count, const GLdouble *value) const
{
    if (_glUniform1dv)
    {

        _glUniform1dv(location, count, value);
    }
    else
    {
        NotSupported( "glUniform1dv" );
    }
}

void GL2Extensions::glUniform2dv(GLint location, GLsizei count, const GLdouble *value) const
{
    if (_glUniform2dv)
    {

        _glUniform2dv(location, count, value);
    }
    else
    {
        NotSupported( "glUniform2dv" );
    }
}

void GL2Extensions::glUniform3dv(GLint location, GLsizei count, const GLdouble *value) const
{
    if (_glUniform3dv)
    {

        _glUniform3dv(location, count, value);
    }
    else
    {
        NotSupported( "glUniform3dv" );
    }
}

void GL2Extensions::glUniform4dv(GLint location, GLsizei count, const GLdouble *value) const
{
    if (_glUniform4dv)
    {

        _glUniform4dv(location, count, value);
    }
    else
    {
        NotSupported( "glUniform4dv" );
    }
}

void GL2Extensions::glUniformMatrix2dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value) const
{
    if (_glUniformMatrix2dv)
    {

        _glUniformMatrix2dv(location, count, transpose, value);
    }
    else
    {
        NotSupported( "glUniformMatrix2dv" );
    }
}

void GL2Extensions::glUniformMatrix3dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value) const
{
    if (_glUniformMatrix3dv)
    {

        _glUniformMatrix3dv(location, count, transpose, value);
    }
    else
    {
        NotSupported( "glUniformMatrix3dv" );
    }
}

void GL2Extensions::glUniformMatrix4dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value) const
{
    if (_glUniformMatrix4dv)
    {

        _glUniformMatrix4dv(location, count, transpose, value);
    }
    else
    {
        NotSupported( "glUniformMatrix4dv" );
    }
}

void GL2Extensions::glUniformMatrix2x3dv( GLint location, GLsizei count, GLboolean transpose, const GLdouble* value ) const
{
    if (_glUniformMatrix2x3dv)
    {

        _glUniformMatrix2x3dv( location, count, transpose, value );
    }
    else
    {
        NotSupported( "glUniformMatrix2x3dv" );
    }
}

void GL2Extensions::glUniformMatrix3x2dv( GLint location, GLsizei count, GLboolean transpose, const GLdouble* value ) const
{
    if (_glUniformMatrix3x2dv)
    {

        _glUniformMatrix3x2dv( location, count, transpose, value );
    }
    else
    {
        NotSupported( "glUniformMatrix3x2dv" );
    }
}

void GL2Extensions::glUniformMatrix2x4dv( GLint location, GLsizei count, GLboolean transpose, const GLdouble* value ) const
{
    if (_glUniformMatrix2x4dv)
    {

        _glUniformMatrix2x4dv( location, count, transpose, value );
    }
    else
    {
        NotSupported( "glUniformMatrix2x4dv" );
    }
}

void GL2Extensions::glUniformMatrix4x2dv( GLint location, GLsizei count, GLboolean transpose, const GLdouble* value ) const
{
    if (_glUniformMatrix4x2dv)
    {

        _glUniformMatrix4x2dv( location, count, transpose, value );
    }
    else
    {
        NotSupported( "glUniformMatrix4x2dv" );
    }
}

void GL2Extensions::glUniformMatrix3x4dv( GLint location, GLsizei count, GLboolean transpose, const GLdouble* value ) const
{
    if (_glUniformMatrix3x4dv)
    {

        _glUniformMatrix3x4dv( location, count, transpose, value );
    }
    else
    {
        NotSupported( "glUniformMatrix3x4dv" );
    }
}

void GL2Extensions::glUniformMatrix4x3dv( GLint location, GLsizei count, GLboolean transpose, const GLdouble* value ) const
{
    if (_glUniformMatrix4x3dv)
    {

        _glUniformMatrix4x3dv( location, count, transpose, value );
    }
    else
    {
        NotSupported( "glUniformMatrix4x3dv" );
    }
}

void GL2Extensions::glGetActiveAtomicCounterBufferiv( GLuint program, GLuint bufferIndex, GLenum pname, GLint* params ) const
{
    if (_glGetActiveAtomicCounterBufferiv)
    {

        _glGetActiveAtomicCounterBufferiv( program, bufferIndex, pname, params );
    }
    else
    {
        NotSupported( "glGetActiveAtomicCounterBufferiv" );
    }
}

void GL2Extensions::glDispatchCompute( GLuint numGroupsX, GLuint numGroupsY, GLuint numGroupsZ ) const
{
    if (_glDispatchCompute)
    {
        _glDispatchCompute( numGroupsX, numGroupsY, numGroupsZ );
    }
    else
    {
        NotSupported( "glDispatchCompute" );
    }
}

///////////////////////////////////////////////////////////////////////////
// C++-friendly convenience methods

GLuint GL2Extensions::getCurrentProgram() const
{
    if( _glVersion >= 2.0f )
    {
        // GLSL as GL v2.0 core functionality
        GLint result = 0;
        glGetIntegerv( GL_CURRENT_PROGRAM, &result );
        return static_cast<GLuint>(result);
    }
    else if (_glGetHandleARB)
    {
        // fallback for GLSL as GL v1.5 ARB extension
#ifndef GL_PROGRAM_OBJECT_ARB
#define GL_PROGRAM_OBJECT_ARB 0x8B40
#endif
        return _glGetHandleARB( GL_PROGRAM_OBJECT_ARB );
    }
    else
    {
        NotSupported( "getCurrentProgram" );
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
    if (_glGetFragDataLocation == NULL) return false;

    // is there such a named attribute?
    GLint loc = glGetFragDataLocation( program, reinterpret_cast<const GLchar*>(fragDataName) );
    if( loc < 0 ) return false;

    location = loc;
    return true;
}

