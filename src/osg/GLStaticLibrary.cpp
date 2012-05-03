/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield
 * Copyright (C) 2003-2005 3Dlabs Inc. Ltd.
 * Copyright (C) 2004-2005 Nathan Cournia
 * Copyright (C) 2008 Zebra Imaging
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

/* file:        src/osg/GLStaticLibrary.cpp
 * author:      Alok Priyadarshi 2010-04-27
*/

#include "GLStaticLibrary.h"
#include <osg/GL>
#include <osg/Notify>

#include <map>
#include <string>

// This file is intended for GL static linking only.
#if defined(OSG_GL_LIBRARY_STATIC)

using namespace osg;

namespace {
typedef void (*GLProc)(void);
typedef std::map<std::string, GLProc> GLProcAddressMap;
static bool sProcAddressInitialized = false;
static GLProcAddressMap sProcAddressMap;

#define ADD_FUNCTION(FunctionName) sProcAddressMap[#FunctionName] = reinterpret_cast<GLProc>(&FunctionName);

void initGLES2ProcAddress()
{
    ADD_FUNCTION(glActiveTexture)
    ADD_FUNCTION(glAttachShader)
    ADD_FUNCTION(glBindAttribLocation)
    ADD_FUNCTION(glBindBuffer)
    ADD_FUNCTION(glBindFramebuffer)
    ADD_FUNCTION(glBindRenderbuffer)
    ADD_FUNCTION(glBindTexture)
    ADD_FUNCTION(glBlendColor)
    ADD_FUNCTION(glBlendEquation)
    ADD_FUNCTION(glBlendEquationSeparate)
    ADD_FUNCTION(glBlendFunc)
    ADD_FUNCTION(glBlendFuncSeparate)
    ADD_FUNCTION(glBufferData)
    ADD_FUNCTION(glBufferSubData)
    ADD_FUNCTION(glCheckFramebufferStatus)
    ADD_FUNCTION(glClear)
    ADD_FUNCTION(glClearColor)
    ADD_FUNCTION(glClearDepthf)
    ADD_FUNCTION(glClearStencil)
    ADD_FUNCTION(glColorMask)
    ADD_FUNCTION(glCompileShader)
    ADD_FUNCTION(glCompressedTexImage2D)
    ADD_FUNCTION(glCompressedTexSubImage2D)
    ADD_FUNCTION(glCopyTexImage2D)
    ADD_FUNCTION(glCopyTexSubImage2D)
    ADD_FUNCTION(glCreateProgram)
    ADD_FUNCTION(glCreateShader)
    ADD_FUNCTION(glCullFace)
    ADD_FUNCTION(glDeleteBuffers)
    ADD_FUNCTION(glDeleteFramebuffers)
    ADD_FUNCTION(glDeleteProgram)
    ADD_FUNCTION(glDeleteRenderbuffers)
    ADD_FUNCTION(glDeleteShader)
    ADD_FUNCTION(glDeleteTextures)
    ADD_FUNCTION(glDepthFunc)
    ADD_FUNCTION(glDepthMask)
    ADD_FUNCTION(glDepthRangef)
    ADD_FUNCTION(glDetachShader)
    ADD_FUNCTION(glDisable)
    ADD_FUNCTION(glDisableVertexAttribArray)
    ADD_FUNCTION(glDrawArrays)
    ADD_FUNCTION(glDrawElements)
    ADD_FUNCTION(glEnable)
    ADD_FUNCTION(glEnableVertexAttribArray)
    ADD_FUNCTION(glFinish)
    ADD_FUNCTION(glFlush)
    ADD_FUNCTION(glFramebufferRenderbuffer)
    ADD_FUNCTION(glFramebufferTexture2D)
    ADD_FUNCTION(glFrontFace)
    ADD_FUNCTION(glGenBuffers)
    ADD_FUNCTION(glGenerateMipmap)
    ADD_FUNCTION(glGenFramebuffers)
    ADD_FUNCTION(glGenRenderbuffers)
    ADD_FUNCTION(glGenTextures)
    ADD_FUNCTION(glGetActiveAttrib)
    ADD_FUNCTION(glGetActiveUniform)
    ADD_FUNCTION(glGetAttachedShaders)
    ADD_FUNCTION(glGetAttribLocation)
    ADD_FUNCTION(glGetBooleanv)
    ADD_FUNCTION(glGetBufferParameteriv)
    ADD_FUNCTION(glGetError)
    ADD_FUNCTION(glGetFloatv)
    ADD_FUNCTION(glGetFramebufferAttachmentParameteriv)
    ADD_FUNCTION(glGetIntegerv)
    ADD_FUNCTION(glGetProgramiv)
    ADD_FUNCTION(glGetProgramInfoLog)
    ADD_FUNCTION(glGetRenderbufferParameteriv)
    ADD_FUNCTION(glGetShaderiv)
    ADD_FUNCTION(glGetShaderInfoLog)
    ADD_FUNCTION(glGetShaderPrecisionFormat)
    ADD_FUNCTION(glGetShaderSource)
    ADD_FUNCTION(glGetString)
    ADD_FUNCTION(glGetTexParameterfv)
    ADD_FUNCTION(glGetTexParameteriv)
    ADD_FUNCTION(glGetUniformfv)
    ADD_FUNCTION(glGetUniformiv)
    ADD_FUNCTION(glGetUniformLocation)
    ADD_FUNCTION(glGetVertexAttribfv)
    ADD_FUNCTION(glGetVertexAttribiv)
    ADD_FUNCTION(glGetVertexAttribPointerv)
    ADD_FUNCTION(glHint)
    ADD_FUNCTION(glIsBuffer)
    ADD_FUNCTION(glIsEnabled)
    ADD_FUNCTION(glIsFramebuffer)
    ADD_FUNCTION(glIsProgram)
    ADD_FUNCTION(glIsRenderbuffer)
    ADD_FUNCTION(glIsShader)
    ADD_FUNCTION(glIsTexture)
    ADD_FUNCTION(glLineWidth)
    ADD_FUNCTION(glLinkProgram)
    ADD_FUNCTION(glPixelStorei)
    ADD_FUNCTION(glPolygonOffset)
    ADD_FUNCTION(glReadPixels)
    ADD_FUNCTION(glReleaseShaderCompiler)
    ADD_FUNCTION(glRenderbufferStorage)
    ADD_FUNCTION(glSampleCoverage)
    ADD_FUNCTION(glScissor)
    ADD_FUNCTION(glShaderBinary)
    ADD_FUNCTION(glShaderSource)
    ADD_FUNCTION(glStencilFunc)
    ADD_FUNCTION(glStencilFuncSeparate)
    ADD_FUNCTION(glStencilMask)
    ADD_FUNCTION(glStencilMaskSeparate)
    ADD_FUNCTION(glStencilOp)
    ADD_FUNCTION(glStencilOpSeparate)
    ADD_FUNCTION(glTexImage2D)
    ADD_FUNCTION(glTexParameterf)
    ADD_FUNCTION(glTexParameterfv)
    ADD_FUNCTION(glTexParameteri)
    ADD_FUNCTION(glTexParameteriv)
    ADD_FUNCTION(glTexSubImage2D)
    ADD_FUNCTION(glUniform1f)
    ADD_FUNCTION(glUniform1fv)
    ADD_FUNCTION(glUniform1i)
    ADD_FUNCTION(glUniform1iv)
    ADD_FUNCTION(glUniform2f)
    ADD_FUNCTION(glUniform2fv)
    ADD_FUNCTION(glUniform2i)
    ADD_FUNCTION(glUniform2iv)
    ADD_FUNCTION(glUniform3f)
    ADD_FUNCTION(glUniform3fv)
    ADD_FUNCTION(glUniform3i)
    ADD_FUNCTION(glUniform3iv)
    ADD_FUNCTION(glUniform4f)
    ADD_FUNCTION(glUniform4fv)
    ADD_FUNCTION(glUniform4i)
    ADD_FUNCTION(glUniform4iv)
    ADD_FUNCTION(glUniformMatrix2fv)
    ADD_FUNCTION(glUniformMatrix3fv)
    ADD_FUNCTION(glUniformMatrix4fv)
    ADD_FUNCTION(glUseProgram)
    ADD_FUNCTION(glValidateProgram)
    ADD_FUNCTION(glVertexAttrib1f)
    ADD_FUNCTION(glVertexAttrib1fv)
    ADD_FUNCTION(glVertexAttrib2f)
    ADD_FUNCTION(glVertexAttrib2fv)
    ADD_FUNCTION(glVertexAttrib3f)
    ADD_FUNCTION(glVertexAttrib3fv)
    ADD_FUNCTION(glVertexAttrib4f)
    ADD_FUNCTION(glVertexAttrib4fv)
    ADD_FUNCTION(glVertexAttribPointer)
    ADD_FUNCTION(glViewport)
}

void initProcAddress()
{
#if defined(OSG_GLES2_AVAILABLE)
    initGLES2ProcAddress();
#else
    OSG_NOTICE << "initProcAddress() not implemented for static GL lib yet." << std::endl;
#endif
}

}  // namespace

void* GLStaticLibrary::getProcAddress(const char* procName)
{
    // TODO(alokp): Add a mutex around sProcAddressInitialized.
    if (!sProcAddressInitialized)
    {
        initProcAddress();
        sProcAddressInitialized = true;
    }

    GLProcAddressMap::const_iterator iter = sProcAddressMap.find(procName);
    return iter != sProcAddressMap.end() ? reinterpret_cast<void*>(iter->second) : 0;
}

#endif  // OSG_GLES2_LIBRARY_STATIC
