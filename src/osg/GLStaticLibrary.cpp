/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield 
 * Copyright (C) 2003-2005 3Dlabs Inc. Ltd.
 * Copyright (C) 2004-2005 Nathan Cournia
 * Copyright (C) 2008 Zebra Imaging
 *
 * This application is open source and may be redistributed and/or modified   
 * freely and without restriction, both in commericial and non commericial
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

void initGLES2ProcAddress()
{
    sProcAddressMap["glActiveTexture"] = reinterpret_cast<GLProc>(&glActiveTexture);
    sProcAddressMap["glAttachShader"] = reinterpret_cast<GLProc>(&glAttachShader);
    sProcAddressMap["glBindAttribLocation"] = reinterpret_cast<GLProc>(&glBindAttribLocation);
    sProcAddressMap["glBindBuffer"] = reinterpret_cast<GLProc>(&glBindBuffer);
    sProcAddressMap["glBindFramebuffer"] = reinterpret_cast<GLProc>(&glBindFramebuffer);
    sProcAddressMap["glBindRenderbuffer"] = reinterpret_cast<GLProc>(&glBindRenderbuffer);
    sProcAddressMap["glBindTexture"] = reinterpret_cast<GLProc>(&glBindTexture);
    sProcAddressMap["glBlendColor"] = reinterpret_cast<GLProc>(&glBlendColor);
    sProcAddressMap["glBlendEquation"] = reinterpret_cast<GLProc>(&glBlendEquation);
    sProcAddressMap["glBlendEquationSeparate"] = reinterpret_cast<GLProc>(&glBlendEquationSeparate);
    sProcAddressMap["glBlendFunc"] = reinterpret_cast<GLProc>(&glBlendFunc);
    sProcAddressMap["glBlendFuncSeparate"] = reinterpret_cast<GLProc>(&glBlendFuncSeparate);
    sProcAddressMap["glBufferData"] = reinterpret_cast<GLProc>(&glBufferData);
    sProcAddressMap["glBufferSubData"] = reinterpret_cast<GLProc>(&glBufferSubData);
    sProcAddressMap["glCheckFramebufferStatus"] = reinterpret_cast<GLProc>(&glCheckFramebufferStatus);
    sProcAddressMap["glClear"] = reinterpret_cast<GLProc>(&glClear);
    sProcAddressMap["glClearColor"] = reinterpret_cast<GLProc>(&glClearColor);
    sProcAddressMap["glClearDepthf"] = reinterpret_cast<GLProc>(&glClearDepthf);
    sProcAddressMap["glClearStencil"] = reinterpret_cast<GLProc>(&glClearStencil);
    sProcAddressMap["glColorMask"] = reinterpret_cast<GLProc>(&glColorMask);
    sProcAddressMap["glCompileShader"] = reinterpret_cast<GLProc>(&glCompileShader);
    sProcAddressMap["glCompressedTexImage2D"] = reinterpret_cast<GLProc>(&glCompressedTexImage2D);
    sProcAddressMap["glCompressedTexSubImage2D"] = reinterpret_cast<GLProc>(&glCompressedTexSubImage2D);
    sProcAddressMap["glCopyTexImage2D"] = reinterpret_cast<GLProc>(&glCopyTexImage2D);
    sProcAddressMap["glCopyTexSubImage2D"] = reinterpret_cast<GLProc>(&glCopyTexSubImage2D);
    sProcAddressMap["glCreateProgram"] = reinterpret_cast<GLProc>(&glCreateProgram);
    sProcAddressMap["glCreateShader"] = reinterpret_cast<GLProc>(&glCreateShader);
    sProcAddressMap["glCullFace"] = reinterpret_cast<GLProc>(&glCullFace);
    sProcAddressMap["glDeleteBuffers"] = reinterpret_cast<GLProc>(&glDeleteBuffers);
    sProcAddressMap["glDeleteFramebuffers"] = reinterpret_cast<GLProc>(&glDeleteFramebuffers);
    sProcAddressMap["glDeleteProgram"] = reinterpret_cast<GLProc>(&glDeleteProgram);
    sProcAddressMap["glDeleteRenderbuffers"] = reinterpret_cast<GLProc>(&glDeleteRenderbuffers);
    sProcAddressMap["glDeleteShader"] = reinterpret_cast<GLProc>(&glDeleteShader);
    sProcAddressMap["glDeleteTextures"] = reinterpret_cast<GLProc>(&glDeleteTextures);
    sProcAddressMap["glDepthFunc"] = reinterpret_cast<GLProc>(&glDepthFunc);
    sProcAddressMap["glDepthMask"] = reinterpret_cast<GLProc>(&glDepthMask);
    sProcAddressMap["glDepthRangef"] = reinterpret_cast<GLProc>(&glDepthRangef);
    sProcAddressMap["glDetachShader"] = reinterpret_cast<GLProc>(&glDetachShader);
    sProcAddressMap["glDisable"] = reinterpret_cast<GLProc>(&glDisable);
    sProcAddressMap["glDisableVertexAttribArray"] = reinterpret_cast<GLProc>(&glDisableVertexAttribArray);
    sProcAddressMap["glDrawArrays"] = reinterpret_cast<GLProc>(&glDrawArrays);
    sProcAddressMap["glDrawElements"] = reinterpret_cast<GLProc>(&glDrawElements);
    sProcAddressMap["glEnable"] = reinterpret_cast<GLProc>(&glEnable);
    sProcAddressMap["glEnableVertexAttribArray"] = reinterpret_cast<GLProc>(&glEnableVertexAttribArray);
    sProcAddressMap["glFinish"] = reinterpret_cast<GLProc>(&glFinish);
    sProcAddressMap["glFlush"] = reinterpret_cast<GLProc>(&glFlush);
    sProcAddressMap["glFramebufferRenderbuffer"] = reinterpret_cast<GLProc>(&glFramebufferRenderbuffer);
    sProcAddressMap["glFramebufferTexture2D"] = reinterpret_cast<GLProc>(&glFramebufferTexture2D);
    sProcAddressMap["glFrontFace"] = reinterpret_cast<GLProc>(&glFrontFace);
    sProcAddressMap["glGenBuffers"] = reinterpret_cast<GLProc>(&glGenBuffers);
    sProcAddressMap["glGenerateMipmap"] = reinterpret_cast<GLProc>(&glGenerateMipmap);
    sProcAddressMap["glGenFramebuffers"] = reinterpret_cast<GLProc>(&glGenFramebuffers);
    sProcAddressMap["glGenRenderbuffers"] = reinterpret_cast<GLProc>(&glGenRenderbuffers);
    sProcAddressMap["glGenTextures"] = reinterpret_cast<GLProc>(&glGenTextures);
    sProcAddressMap["glGetActiveAttrib"] = reinterpret_cast<GLProc>(&glGetActiveAttrib);
    sProcAddressMap["glGetActiveUniform"] = reinterpret_cast<GLProc>(&glGetActiveUniform);
    sProcAddressMap["glGetAttachedShaders"] = reinterpret_cast<GLProc>(&glGetAttachedShaders);
    sProcAddressMap["glGetAttribLocation"] = reinterpret_cast<GLProc>(&glGetAttribLocation);
    sProcAddressMap["glGetBooleanv"] = reinterpret_cast<GLProc>(&glGetBooleanv);
    sProcAddressMap["glGetBufferParameteriv"] = reinterpret_cast<GLProc>(&glGetBufferParameteriv);
    sProcAddressMap["glGetError"] = reinterpret_cast<GLProc>(&glGetError);
    sProcAddressMap["glGetFloatv"] = reinterpret_cast<GLProc>(&glGetFloatv);
    sProcAddressMap["glGetFramebufferAttachmentParameteriv"] = reinterpret_cast<GLProc>(&glGetFramebufferAttachmentParameteriv);
    sProcAddressMap["glGetIntegerv"] = reinterpret_cast<GLProc>(&glGetIntegerv);
    sProcAddressMap["glGetProgramiv"] = reinterpret_cast<GLProc>(&glGetProgramiv);
    sProcAddressMap["glGetProgramInfoLog"] = reinterpret_cast<GLProc>(&glGetProgramInfoLog);
    sProcAddressMap["glGetRenderbufferParameteriv"] = reinterpret_cast<GLProc>(&glGetRenderbufferParameteriv);
    sProcAddressMap["glGetShaderiv"] = reinterpret_cast<GLProc>(&glGetShaderiv);
    sProcAddressMap["glGetShaderInfoLog"] = reinterpret_cast<GLProc>(&glGetShaderInfoLog);
    sProcAddressMap["glGetShaderPrecisionFormat"] = reinterpret_cast<GLProc>(&glGetShaderPrecisionFormat);
    sProcAddressMap["glGetShaderSource"] = reinterpret_cast<GLProc>(&glGetShaderSource);
    sProcAddressMap["glGetString"] = reinterpret_cast<GLProc>(&glGetString);
    sProcAddressMap["glGetTexParameterfv"] = reinterpret_cast<GLProc>(&glGetTexParameterfv);
    sProcAddressMap["glGetTexParameteriv"] = reinterpret_cast<GLProc>(&glGetTexParameteriv);
    sProcAddressMap["glGetUniformfv"] = reinterpret_cast<GLProc>(&glGetUniformfv);
    sProcAddressMap["glGetUniformiv"] = reinterpret_cast<GLProc>(&glGetUniformiv);
    sProcAddressMap["glGetUniformLocation"] = reinterpret_cast<GLProc>(&glGetUniformLocation);
    sProcAddressMap["glGetVertexAttribfv"] = reinterpret_cast<GLProc>(&glGetVertexAttribfv);
    sProcAddressMap["glGetVertexAttribiv"] = reinterpret_cast<GLProc>(&glGetVertexAttribiv);
    sProcAddressMap["glGetVertexAttribPointerv"] = reinterpret_cast<GLProc>(&glGetVertexAttribPointerv);
    sProcAddressMap["glHint"] = reinterpret_cast<GLProc>(&glHint);
    sProcAddressMap["glIsBuffer"] = reinterpret_cast<GLProc>(&glIsBuffer);
    sProcAddressMap["glIsEnabled"] = reinterpret_cast<GLProc>(&glIsEnabled);
    sProcAddressMap["glIsFramebuffer"] = reinterpret_cast<GLProc>(&glIsFramebuffer);
    sProcAddressMap["glIsProgram"] = reinterpret_cast<GLProc>(&glIsProgram);
    sProcAddressMap["glIsRenderbuffer"] = reinterpret_cast<GLProc>(&glIsRenderbuffer);
    sProcAddressMap["glIsShader"] = reinterpret_cast<GLProc>(&glIsShader);
    sProcAddressMap["glIsTexture"] = reinterpret_cast<GLProc>(&glIsTexture);
    sProcAddressMap["glLineWidth"] = reinterpret_cast<GLProc>(&glLineWidth);
    sProcAddressMap["glLinkProgram"] = reinterpret_cast<GLProc>(&glLinkProgram);
    sProcAddressMap["glPixelStorei"] = reinterpret_cast<GLProc>(&glPixelStorei);
    sProcAddressMap["glPolygonOffset"] = reinterpret_cast<GLProc>(&glPolygonOffset);
    sProcAddressMap["glReadPixels"] = reinterpret_cast<GLProc>(&glReadPixels);
    sProcAddressMap["glReleaseShaderCompiler"] = reinterpret_cast<GLProc>(&glReleaseShaderCompiler);
    sProcAddressMap["glRenderbufferStorage"] = reinterpret_cast<GLProc>(&glRenderbufferStorage);
    sProcAddressMap["glSampleCoverage"] = reinterpret_cast<GLProc>(&glSampleCoverage);
    sProcAddressMap["glScissor"] = reinterpret_cast<GLProc>(&glScissor);
    sProcAddressMap["glShaderBinary"] = reinterpret_cast<GLProc>(&glShaderBinary);
    sProcAddressMap["glShaderSource"] = reinterpret_cast<GLProc>(&glShaderSource);
    sProcAddressMap["glStencilFunc"] = reinterpret_cast<GLProc>(&glStencilFunc);
    sProcAddressMap["glStencilFuncSeparate"] = reinterpret_cast<GLProc>(&glStencilFuncSeparate);
    sProcAddressMap["glStencilMask"] = reinterpret_cast<GLProc>(&glStencilMask);
    sProcAddressMap["glStencilMaskSeparate"] = reinterpret_cast<GLProc>(&glStencilMaskSeparate);
    sProcAddressMap["glStencilOp"] = reinterpret_cast<GLProc>(&glStencilOp);
    sProcAddressMap["glStencilOpSeparate"] = reinterpret_cast<GLProc>(&glStencilOpSeparate);
    sProcAddressMap["glTexImage2D"] = reinterpret_cast<GLProc>(&glTexImage2D);
    sProcAddressMap["glTexParameterf"] = reinterpret_cast<GLProc>(&glTexParameterf);
    sProcAddressMap["glTexParameterfv"] = reinterpret_cast<GLProc>(&glTexParameterfv);
    sProcAddressMap["glTexParameteri"] = reinterpret_cast<GLProc>(&glTexParameteri);
    sProcAddressMap["glTexParameteriv"] = reinterpret_cast<GLProc>(&glTexParameteriv);
    sProcAddressMap["glTexSubImage2D"] = reinterpret_cast<GLProc>(&glTexSubImage2D);
    sProcAddressMap["glUniform1f"] = reinterpret_cast<GLProc>(&glUniform1f);
    sProcAddressMap["glUniform1fv"] = reinterpret_cast<GLProc>(&glUniform1fv);
    sProcAddressMap["glUniform1i"] = reinterpret_cast<GLProc>(&glUniform1i);
    sProcAddressMap["glUniform1iv"] = reinterpret_cast<GLProc>(&glUniform1iv);
    sProcAddressMap["glUniform2f"] = reinterpret_cast<GLProc>(&glUniform2f);
    sProcAddressMap["glUniform2fv"] = reinterpret_cast<GLProc>(&glUniform2fv);
    sProcAddressMap["glUniform2i"] = reinterpret_cast<GLProc>(&glUniform2i);
    sProcAddressMap["glUniform2iv"] = reinterpret_cast<GLProc>(&glUniform2iv);
    sProcAddressMap["glUniform3f"] = reinterpret_cast<GLProc>(&glUniform3f);
    sProcAddressMap["glUniform3fv"] = reinterpret_cast<GLProc>(&glUniform3fv);
    sProcAddressMap["glUniform3i"] = reinterpret_cast<GLProc>(&glUniform3i);
    sProcAddressMap["glUniform3iv"] = reinterpret_cast<GLProc>(&glUniform3iv);
    sProcAddressMap["glUniform4f"] = reinterpret_cast<GLProc>(&glUniform4f);
    sProcAddressMap["glUniform4fv"] = reinterpret_cast<GLProc>(&glUniform4fv);
    sProcAddressMap["glUniform4i"] = reinterpret_cast<GLProc>(&glUniform4i);
    sProcAddressMap["glUniform4iv"] = reinterpret_cast<GLProc>(&glUniform4iv);
    sProcAddressMap["glUniformMatrix2fv"] = reinterpret_cast<GLProc>(&glUniformMatrix2fv);
    sProcAddressMap["glUniformMatrix3fv"] = reinterpret_cast<GLProc>(&glUniformMatrix3fv);
    sProcAddressMap["glUniformMatrix4fv"] = reinterpret_cast<GLProc>(&glUniformMatrix4fv);
    sProcAddressMap["glUseProgram"] = reinterpret_cast<GLProc>(&glUseProgram);
    sProcAddressMap["glValidateProgram"] = reinterpret_cast<GLProc>(&glValidateProgram);
    sProcAddressMap["glVertexAttrib1f"] = reinterpret_cast<GLProc>(&glVertexAttrib1f);
    sProcAddressMap["glVertexAttrib1fv"] = reinterpret_cast<GLProc>(&glVertexAttrib1fv);
    sProcAddressMap["glVertexAttrib2f"] = reinterpret_cast<GLProc>(&glVertexAttrib2f);
    sProcAddressMap["glVertexAttrib2fv"] = reinterpret_cast<GLProc>(&glVertexAttrib2fv);
    sProcAddressMap["glVertexAttrib3f"] = reinterpret_cast<GLProc>(&glVertexAttrib3f);
    sProcAddressMap["glVertexAttrib3fv"] = reinterpret_cast<GLProc>(&glVertexAttrib3fv);
    sProcAddressMap["glVertexAttrib4f"] = reinterpret_cast<GLProc>(&glVertexAttrib4f);
    sProcAddressMap["glVertexAttrib4fv"] = reinterpret_cast<GLProc>(&glVertexAttrib4fv);
    sProcAddressMap["glVertexAttribPointer"] = reinterpret_cast<GLProc>(&glVertexAttribPointer);
    sProcAddressMap["glViewport"] = reinterpret_cast<GLProc>(&glViewport);
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
    return iter != sProcAddressMap.end() ? iter->second : 0;
}

#endif  // OSG_GLES2_LIBRARY_STATIC

