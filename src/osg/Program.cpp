/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield 
 * Copyright (C) 2003-2005 3Dlabs Inc. Ltd.
 * Copyright (C) 2004-2005 Nathan Cournia
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

/* file:        src/osg/Program.cpp
 * author:      Mike Weiblen 2006-03-25
*/

#include <fstream>
#include <list>

#include <osg/Notify>
#include <osg/State>
#include <osg/Timer>
#include <osg/buffered_value>
#include <osg/ref_ptr>
#include <osg/Program>
#include <osg/Shader>
#include <osg/Uniform>
#include <osg/GLExtensions>
#include <osg/GLU>

#include <OpenThreads/ScopedLock>
#include <OpenThreads/Mutex>

using namespace osg;


///////////////////////////////////////////////////////////////////////////
// Extension function pointers for OpenGL v2.0

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

    _glGetInfoLogARB = rhs._glGetInfoLogARB;
    _glGetObjectParameterivARB = rhs._glGetObjectParameterivARB;
    _glDeleteObjectARB = rhs._glDeleteObjectARB;
    _glGetHandleARB = rhs._glGetHandleARB;
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

    if (!rhs._glGetInfoLogARB) _glGetInfoLogARB = 0;
    if (!rhs._glGetObjectParameterivARB) _glGetObjectParameterivARB = 0;
    if (!rhs._glDeleteObjectARB) _glDeleteObjectARB = 0;
    if (!rhs._glGetHandleARB) _glGetHandleARB = 0;
}


void GL2Extensions::setupGL2Extensions(unsigned int contextID)
{
    const char* version = (const char*) glGetString( GL_VERSION );
    if (!version)
    {
        osg::notify(osg::FATAL)<<"Error: OpenGL version test failed, requires valid graphics context."<<std::endl;
        return;
    }
    
    _glVersion = atof( version );
    _glslLanguageVersion = 0.0f;

    _isShaderObjectsSupported = osg::isGLExtensionSupported(contextID,"GL_ARB_shader_objects");
    _isVertexShaderSupported = osg::isGLExtensionSupported(contextID,"GL_ARB_vertex_shader");
    _isFragmentShaderSupported = osg::isGLExtensionSupported(contextID,"GL_ARB_fragment_shader");
    _isLanguage100Supported = osg::isGLExtensionSupported(contextID,"GL_ARB_shading_language_100");

    if( isGlslSupported() )
    {
        // If glGetString raises an error, assume initial release "1.00"
        while(glGetError() != GL_NO_ERROR) {}        // reset error flag
        const char* langVerStr = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
        if( (glGetError() == GL_NO_ERROR) && langVerStr )
            _glslLanguageVersion = atof( langVerStr );
        else
            _glslLanguageVersion = 1.0f;
    }

    osg::notify(osg::INFO)
            << "glVersion=" << getGlVersion() << ", "
            << "isGlslSupported=" << (isGlslSupported() ? "YES" : "NO") << ", "
            << "glslLanguageVersion=" << getLanguageVersion()
            << std::endl;


    _glBlendEquationSeparate = osg::getGLExtensionFuncPtr("glBlendEquationSeparate");
    _glDrawBuffers = osg::getGLExtensionFuncPtr("glDrawBuffers", "glDrawBuffersARB");
    _glStencilOpSeparate = osg::getGLExtensionFuncPtr("glStencilOpSeparate");
    _glStencilFuncSeparate = osg::getGLExtensionFuncPtr("glStencilFuncSeparate");
    _glStencilMaskSeparate = osg::getGLExtensionFuncPtr("glStencilMaskSeparate");
    _glAttachShader = osg::getGLExtensionFuncPtr("glAttachShader", "glAttachObjectARB");
    _glBindAttribLocation = osg::getGLExtensionFuncPtr("glBindAttribLocation", "glBindAttribLocationARB");
    _glCompileShader = osg::getGLExtensionFuncPtr("glCompileShader", "glCompileShaderARB");
    _glCreateProgram = osg::getGLExtensionFuncPtr("glCreateProgram", "glCreateProgramObjectARB");
    _glCreateShader = osg::getGLExtensionFuncPtr("glCreateShader", "glCreateShaderObjectARB");
    _glDeleteProgram = osg::getGLExtensionFuncPtr("glDeleteProgram");
    _glDeleteShader = osg::getGLExtensionFuncPtr("glDeleteShader");
    _glDetachShader = osg::getGLExtensionFuncPtr("glDetachShader", "glDetachObjectARB");
    _glDisableVertexAttribArray = osg::getGLExtensionFuncPtr("glDisableVertexAttribArray");
    _glEnableVertexAttribArray = osg::getGLExtensionFuncPtr("glEnableVertexAttribArray");
    _glGetActiveAttrib = osg::getGLExtensionFuncPtr("glGetActiveAttrib", "glGetActiveAttribARB");
    _glGetActiveUniform = osg::getGLExtensionFuncPtr("glGetActiveUniform", "glGetActiveUniformARB");
    _glGetAttachedShaders = osg::getGLExtensionFuncPtr("glGetAttachedShaders", "glGetAttachedObjectsARB");
    _glGetAttribLocation = osg::getGLExtensionFuncPtr("glGetAttribLocation", "glGetAttribLocationARB");
    _glGetProgramiv = osg::getGLExtensionFuncPtr("glGetProgramiv");
    _glGetProgramInfoLog = osg::getGLExtensionFuncPtr("glGetProgramInfoLog");
    _glGetShaderiv = osg::getGLExtensionFuncPtr("glGetShaderiv");
    _glGetShaderInfoLog = osg::getGLExtensionFuncPtr("glGetShaderInfoLog");
    _glGetShaderSource = osg::getGLExtensionFuncPtr("glGetShaderSource", "glGetShaderSourceARB");
    _glGetUniformLocation = osg::getGLExtensionFuncPtr("glGetUniformLocation", "glGetUniformLocationARB");
    _glGetUniformfv = osg::getGLExtensionFuncPtr("glGetUniformfv", "glGetUniformfvARB");
    _glGetUniformiv = osg::getGLExtensionFuncPtr("glGetUniformiv", "glGetUniformivARB");
    _glGetVertexAttribdv = osg::getGLExtensionFuncPtr("glGetVertexAttribdv");
    _glGetVertexAttribfv = osg::getGLExtensionFuncPtr("glGetVertexAttribfv");
    _glGetVertexAttribiv = osg::getGLExtensionFuncPtr("glGetVertexAttribiv");
    _glGetVertexAttribPointerv = osg::getGLExtensionFuncPtr("glGetVertexAttribPointerv");
    _glIsProgram = osg::getGLExtensionFuncPtr("glIsProgram");
    _glIsShader = osg::getGLExtensionFuncPtr("glIsShader");
    _glLinkProgram = osg::getGLExtensionFuncPtr("glLinkProgram", "glLinkProgramARB");
    _glShaderSource = osg::getGLExtensionFuncPtr("glShaderSource", "glShaderSourceARB");
    _glUseProgram = osg::getGLExtensionFuncPtr("glUseProgram", "glUseProgramObjectARB");
    _glUniform1f = osg::getGLExtensionFuncPtr("glUniform1f", "glUniform1fARB");
    _glUniform2f = osg::getGLExtensionFuncPtr("glUniform2f", "glUniform2fARB");
    _glUniform3f = osg::getGLExtensionFuncPtr("glUniform3f", "glUniform3fARB");
    _glUniform4f = osg::getGLExtensionFuncPtr("glUniform4f", "glUniform4fARB");
    _glUniform1i = osg::getGLExtensionFuncPtr("glUniform1i", "glUniform1iARB");
    _glUniform2i = osg::getGLExtensionFuncPtr("glUniform2i", "glUniform2iARB");
    _glUniform3i = osg::getGLExtensionFuncPtr("glUniform3i", "glUniform3iARB");
    _glUniform4i = osg::getGLExtensionFuncPtr("glUniform4i", "glUniform4iARB");
    _glUniform1fv = osg::getGLExtensionFuncPtr("glUniform1fv", "glUniform1fvARB");
    _glUniform2fv = osg::getGLExtensionFuncPtr("glUniform2fv", "glUniform2fvARB");
    _glUniform3fv = osg::getGLExtensionFuncPtr("glUniform3fv", "glUniform3fvARB");
    _glUniform4fv = osg::getGLExtensionFuncPtr("glUniform4fv", "glUniform4fvARB");
    _glUniform1iv = osg::getGLExtensionFuncPtr("glUniform1iv", "glUniform1ivARB");
    _glUniform2iv = osg::getGLExtensionFuncPtr("glUniform2iv", "glUniform2ivARB");
    _glUniform3iv = osg::getGLExtensionFuncPtr("glUniform3iv", "glUniform3ivARB");
    _glUniform4iv = osg::getGLExtensionFuncPtr("glUniform4iv", "glUniform4ivARB");
    _glUniformMatrix2fv = osg::getGLExtensionFuncPtr("glUniformMatrix2fv", "glUniformMatrix2fvARB");
    _glUniformMatrix3fv = osg::getGLExtensionFuncPtr("glUniformMatrix3fv", "glUniformMatrix3fvARB");
    _glUniformMatrix4fv = osg::getGLExtensionFuncPtr("glUniformMatrix4fv", "glUniformMatrix4fvARB");
    _glValidateProgram = osg::getGLExtensionFuncPtr("glValidateProgram", "glValidateProgramARB");
    _glVertexAttrib1d = osg::getGLExtensionFuncPtr("glVertexAttrib1d");
    _glVertexAttrib1dv = osg::getGLExtensionFuncPtr("glVertexAttrib1dv");
    _glVertexAttrib1f = osg::getGLExtensionFuncPtr("glVertexAttrib1f");
    _glVertexAttrib1fv = osg::getGLExtensionFuncPtr("glVertexAttrib1fv");
    _glVertexAttrib1s = osg::getGLExtensionFuncPtr("glVertexAttrib1s");
    _glVertexAttrib1sv = osg::getGLExtensionFuncPtr("glVertexAttrib1sv");
    _glVertexAttrib2d = osg::getGLExtensionFuncPtr("glVertexAttrib2d");
    _glVertexAttrib2dv = osg::getGLExtensionFuncPtr("glVertexAttrib2dv");
    _glVertexAttrib2f = osg::getGLExtensionFuncPtr("glVertexAttrib2f");
    _glVertexAttrib2fv = osg::getGLExtensionFuncPtr("glVertexAttrib2fv");
    _glVertexAttrib2s = osg::getGLExtensionFuncPtr("glVertexAttrib2s");
    _glVertexAttrib2sv = osg::getGLExtensionFuncPtr("glVertexAttrib2sv");
    _glVertexAttrib3d = osg::getGLExtensionFuncPtr("glVertexAttrib3d");
    _glVertexAttrib3dv = osg::getGLExtensionFuncPtr("glVertexAttrib3dv");
    _glVertexAttrib3f = osg::getGLExtensionFuncPtr("glVertexAttrib3f");
    _glVertexAttrib3fv = osg::getGLExtensionFuncPtr("glVertexAttrib3fv");
    _glVertexAttrib3s = osg::getGLExtensionFuncPtr("glVertexAttrib3s");
    _glVertexAttrib3sv = osg::getGLExtensionFuncPtr("glVertexAttrib3sv");
    _glVertexAttrib4Nbv = osg::getGLExtensionFuncPtr("glVertexAttrib4Nbv");
    _glVertexAttrib4Niv = osg::getGLExtensionFuncPtr("glVertexAttrib4Niv");
    _glVertexAttrib4Nsv = osg::getGLExtensionFuncPtr("glVertexAttrib4Nsv");
    _glVertexAttrib4Nub = osg::getGLExtensionFuncPtr("glVertexAttrib4Nub");
    _glVertexAttrib4Nubv = osg::getGLExtensionFuncPtr("glVertexAttrib4Nubv");
    _glVertexAttrib4Nuiv = osg::getGLExtensionFuncPtr("glVertexAttrib4Nuiv");
    _glVertexAttrib4Nusv = osg::getGLExtensionFuncPtr("glVertexAttrib4Nusv");
    _glVertexAttrib4bv = osg::getGLExtensionFuncPtr("glVertexAttrib4bv");
    _glVertexAttrib4d = osg::getGLExtensionFuncPtr("glVertexAttrib4d");
    _glVertexAttrib4dv = osg::getGLExtensionFuncPtr("glVertexAttrib4dv");
    _glVertexAttrib4f = osg::getGLExtensionFuncPtr("glVertexAttrib4f");
    _glVertexAttrib4fv = osg::getGLExtensionFuncPtr("glVertexAttrib4fv");
    _glVertexAttrib4iv = osg::getGLExtensionFuncPtr("glVertexAttrib4iv");
    _glVertexAttrib4s = osg::getGLExtensionFuncPtr("glVertexAttrib4s");
    _glVertexAttrib4sv = osg::getGLExtensionFuncPtr("glVertexAttrib4sv");
    _glVertexAttrib4ubv = osg::getGLExtensionFuncPtr("glVertexAttrib4ubv");
    _glVertexAttrib4uiv = osg::getGLExtensionFuncPtr("glVertexAttrib4uiv");
    _glVertexAttrib4usv = osg::getGLExtensionFuncPtr("glVertexAttrib4usv");
    _glVertexAttribPointer = osg::getGLExtensionFuncPtr("glVertexAttribPointer");

    // v1.5-only ARB entry points, in case they're needed for fallback
    _glGetInfoLogARB = osg::getGLExtensionFuncPtr("glGetInfoLogARB");
    _glGetObjectParameterivARB = osg::getGLExtensionFuncPtr("glGetObjectParameterivARB");
    _glDeleteObjectARB = osg::getGLExtensionFuncPtr("glDeleteObjectARB");
    _glGetHandleARB = osg::getGLExtensionFuncPtr("glGetHandleARB");
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
    osg::notify(osg::WARN)
        <<"Error: "<<funcName<<" not supported by OpenGL driver"<<std::endl;
}



void GL2Extensions::glBlendEquationSeparate(GLenum modeRGB, GLenum modeAlpha) const
{
    if (_glBlendEquationSeparate)
    {
        typedef void (APIENTRY * BlendEquationSeparateProc)(GLenum modeRGB, GLenum modeAlpha);
        ((BlendEquationSeparateProc)_glBlendEquationSeparate)(modeRGB, modeAlpha);
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
        typedef void (APIENTRY * DrawBuffersProc)(GLsizei n, const GLenum *bufs);
        ((DrawBuffersProc)_glDrawBuffers)(n, bufs);
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
        typedef void (APIENTRY * StencilOpSeparateProc)(GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass);
        ((StencilOpSeparateProc)_glStencilOpSeparate)(face, sfail, dpfail, dppass);
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
        typedef void (APIENTRY * StencilFuncSeparateProc)(GLenum frontfunc, GLenum backfunc, GLint ref, GLuint mask);
        ((StencilFuncSeparateProc)_glStencilFuncSeparate)(frontfunc, backfunc, ref, mask);
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
        typedef void (APIENTRY * StencilMaskSeparateProc)(GLenum face, GLuint mask);
        ((StencilMaskSeparateProc)_glStencilMaskSeparate)(face, mask);
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
        typedef void (APIENTRY * AttachShaderProc)(GLuint program, GLuint shader);
        ((AttachShaderProc)_glAttachShader)(program, shader);
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
        typedef void (APIENTRY * BindAttribLocationProc)(GLuint program, GLuint index, const GLchar *name);
        ((BindAttribLocationProc)_glBindAttribLocation)(program, index, name);
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
        typedef void (APIENTRY * CompileShaderProc)(GLuint shader);
        ((CompileShaderProc)_glCompileShader)(shader);
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
        typedef GLuint (APIENTRY * CreateProgramProc)(void);
        return ((CreateProgramProc)_glCreateProgram)();
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
        typedef GLuint (APIENTRY * CreateShaderProc)(GLenum type);
        return ((CreateShaderProc)_glCreateShader)(type);
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
        typedef void (APIENTRY * DeleteProgramProc)(GLuint program);
        ((DeleteProgramProc)_glDeleteProgram)(program);
    }
    else if (_glDeleteObjectARB)
    {
        typedef void (APIENTRY * DeleteObjectARBProc)(GLuint program);
        ((DeleteObjectARBProc)_glDeleteObjectARB)(program);
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
        typedef void (APIENTRY * DeleteShaderProc)(GLuint shader);
        ((DeleteShaderProc)_glDeleteShader)(shader);
    }
    else if (_glDeleteObjectARB)
    {
        typedef void (APIENTRY * DeleteObjectARBProc)(GLuint shader);
        ((DeleteObjectARBProc)_glDeleteObjectARB)(shader);
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
        typedef void (APIENTRY * DetachShaderProc)(GLuint program, GLuint shader);
        ((DetachShaderProc)_glDetachShader)(program, shader);
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
        typedef void (APIENTRY * DisableVertexAttribArrayProc)(GLuint index);
        ((DisableVertexAttribArrayProc)_glDisableVertexAttribArray)(index);
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
        typedef void (APIENTRY * EnableVertexAttribArrayProc)(GLuint index);
        ((EnableVertexAttribArrayProc)_glEnableVertexAttribArray)(index);
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
        typedef void (APIENTRY * GetActiveAttribProc)(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
        ((GetActiveAttribProc)_glGetActiveAttrib)(program, index, bufSize, length, size, type, name);
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
        typedef void (APIENTRY * GetActiveUniformProc)(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
        ((GetActiveUniformProc)_glGetActiveUniform)(program, index, bufSize, length, size, type, name);
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
        typedef void (APIENTRY * GetAttachedShadersProc)(GLuint program, GLsizei maxCount, GLsizei *count, GLuint *obj);
        ((GetAttachedShadersProc)_glGetAttachedShaders)(program, maxCount, count, obj);
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
        typedef GLint (APIENTRY * GetAttribLocationProc)(GLuint program, const GLchar *name);
        return ((GetAttribLocationProc)_glGetAttribLocation)(program, name);
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
        typedef void (APIENTRY * GetProgramivProc)(GLuint program, GLenum pname, GLint *params);
        ((GetProgramivProc)_glGetProgramiv)(program, pname, params);
    }
    else if (_glGetObjectParameterivARB)
    {
        typedef void (APIENTRY * GetObjectParameterivARBProc)(GLuint program, GLenum pname, GLint *params);
        ((GetObjectParameterivARBProc)_glGetObjectParameterivARB)(program, pname, params);
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
        typedef void (APIENTRY * GetProgramInfoLogProc)(GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
        ((GetProgramInfoLogProc)_glGetProgramInfoLog)(program, bufSize, length, infoLog);
    }
    else if (_glGetInfoLogARB)
    {
        typedef void (APIENTRY * GetInfoLogARBProc)(GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
        ((GetInfoLogARBProc)_glGetInfoLogARB)(program, bufSize, length, infoLog);
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
        typedef void (APIENTRY * GetShaderivProc)(GLuint shader, GLenum pname, GLint *params);
        ((GetShaderivProc)_glGetShaderiv)(shader, pname, params);
    }
    else if (_glGetObjectParameterivARB)
    {
        typedef void (APIENTRY * GetObjectParameterivARBProc)(GLuint shader, GLenum pname, GLint *params);
        ((GetObjectParameterivARBProc)_glGetObjectParameterivARB)(shader, pname, params);
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
        typedef void (APIENTRY * GetShaderInfoLogProc)(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
        ((GetShaderInfoLogProc)_glGetShaderInfoLog)(shader, bufSize, length, infoLog);
    }
    else if (_glGetInfoLogARB)
    {
        typedef void (APIENTRY * GetInfoLogARBProc)(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
        ((GetInfoLogARBProc)_glGetInfoLogARB)(shader, bufSize, length, infoLog);
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
        typedef void (APIENTRY * GetShaderSourceProc)(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source);
        ((GetShaderSourceProc)_glGetShaderSource)(shader, bufSize, length, source);
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
        typedef GLint (APIENTRY * GetUniformLocationProc)(GLuint program, const GLchar *name);
        return ((GetUniformLocationProc)_glGetUniformLocation)(program, name);
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
        typedef void (APIENTRY * GetUniformfvProc)(GLuint program, GLint location, GLfloat *params);
        ((GetUniformfvProc)_glGetUniformfv)(program, location, params);
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
        typedef void (APIENTRY * GetUniformivProc)(GLuint program, GLint location, GLint *params);
        ((GetUniformivProc)_glGetUniformiv)(program, location, params);
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
        typedef void (APIENTRY * GetVertexAttribdvProc)(GLuint index, GLenum pname, GLdouble *params);
        ((GetVertexAttribdvProc)_glGetVertexAttribdv)(index, pname, params);
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
        typedef void (APIENTRY * GetVertexAttribfvProc)(GLuint index, GLenum pname, GLfloat *params);
        ((GetVertexAttribfvProc)_glGetVertexAttribfv)(index, pname, params);
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
        typedef void (APIENTRY * GetVertexAttribivProc)(GLuint index, GLenum pname, GLint *params);
        ((GetVertexAttribivProc)_glGetVertexAttribiv)(index, pname, params);
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
        typedef void (APIENTRY * GetVertexAttribPointervProc)(GLuint index, GLenum pname, GLvoid* *pointer);
        ((GetVertexAttribPointervProc)_glGetVertexAttribPointerv)(index, pname, pointer);
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
        typedef GLboolean (APIENTRY * IsProgramProc)(GLuint program);
        return ((IsProgramProc)_glIsProgram)(program);
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
        typedef GLboolean (APIENTRY * IsShaderProc)(GLuint shader);
        return ((IsShaderProc)_glIsShader)(shader);
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
        typedef void (APIENTRY * LinkProgramProc)(GLuint program);
        ((LinkProgramProc)_glLinkProgram)(program);
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
        typedef void (APIENTRY * ShaderSourceProc)(GLuint shader, GLsizei count, const GLchar* *string, const GLint *length);
        ((ShaderSourceProc)_glShaderSource)(shader, count, string, length);
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
        typedef void (APIENTRY * UseProgramProc)(GLuint program);
        ((UseProgramProc)_glUseProgram)(program);
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
        typedef void (APIENTRY * Uniform1fProc)(GLint location, GLfloat v0);
        ((Uniform1fProc)_glUniform1f)(location, v0);
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
        typedef void (APIENTRY * Uniform2fProc)(GLint location, GLfloat v0, GLfloat v1);
        ((Uniform2fProc)_glUniform2f)(location, v0, v1);
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
        typedef void (APIENTRY * Uniform3fProc)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
        ((Uniform3fProc)_glUniform3f)(location, v0, v1, v2);
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
        typedef void (APIENTRY * Uniform4fProc)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
        ((Uniform4fProc)_glUniform4f)(location, v0, v1, v2, v3);
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
        typedef void (APIENTRY * Uniform1iProc)(GLint location, GLint v0);
        ((Uniform1iProc)_glUniform1i)(location, v0);
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
        typedef void (APIENTRY * Uniform2iProc)(GLint location, GLint v0, GLint v1);
        ((Uniform2iProc)_glUniform2i)(location, v0, v1);
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
        typedef void (APIENTRY * Uniform3iProc)(GLint location, GLint v0, GLint v1, GLint v2);
        ((Uniform3iProc)_glUniform3i)(location, v0, v1, v2);
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
        typedef void (APIENTRY * glUniform4iProc)(GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
        ((glUniform4iProc)_glUniform4i)(location, v0, v1, v2, v3);
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
        typedef void (APIENTRY * Uniform1fvProc)(GLint location, GLsizei count, const GLfloat *value);
        ((Uniform1fvProc)_glUniform1fv)(location, count, value);
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
        typedef void (APIENTRY * Uniform2fvProc)(GLint location, GLsizei count, const GLfloat *value);
        ((Uniform2fvProc)_glUniform2fv)(location, count, value);
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
        typedef void (APIENTRY * Uniform3fvProc)(GLint location, GLsizei count, const GLfloat *value);
        ((Uniform3fvProc)_glUniform3fv)(location, count, value);
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
        typedef void (APIENTRY * Uniform4fvProc)(GLint location, GLsizei count, const GLfloat *value);
        ((Uniform4fvProc)_glUniform4fv)(location, count, value);
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
        typedef void (APIENTRY * Uniform1ivProc)(GLint location, GLsizei count, const GLint *value);
        ((Uniform1ivProc)_glUniform1iv)(location, count, value);
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
        typedef void (APIENTRY * Uniform2ivProc)(GLint location, GLsizei count, const GLint *value);
        ((Uniform2ivProc)_glUniform2iv)(location, count, value);
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
        typedef void (APIENTRY * glUniform3ivProc)(GLint location, GLsizei count, const GLint *value);
        ((glUniform3ivProc)_glUniform3iv)(location, count, value);
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
        typedef void (APIENTRY * Uniform4ivProc)(GLint location, GLsizei count, const GLint *value);
        ((Uniform4ivProc)_glUniform4iv)(location, count, value);
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
        typedef void (APIENTRY * UniformMatrix2fvProc)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
        ((UniformMatrix2fvProc)_glUniformMatrix2fv)(location, count, transpose, value);
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
        typedef void (APIENTRY * glUniformMatrix3fvProc)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
        ((glUniformMatrix3fvProc)_glUniformMatrix3fv)(location, count, transpose, value);
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
        typedef void (APIENTRY * UniformMatrix4fvProc)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
        ((UniformMatrix4fvProc)_glUniformMatrix4fv)(location, count, transpose, value);
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
        typedef void (APIENTRY * ValidateProgramProc)(GLuint program);
        ((ValidateProgramProc)_glValidateProgram)(program);
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
        typedef void (APIENTRY * VertexAttrib1dProc)(GLuint index, GLdouble x);
        ((VertexAttrib1dProc)_glVertexAttrib1d)(index, x);
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
        typedef void (APIENTRY * glVertexAttrib1dvProc)(GLuint index, const GLdouble *v);
        ((glVertexAttrib1dvProc)_glVertexAttrib1dv)(index, v);
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
        typedef void (APIENTRY * VertexAttrib1fProc)(GLuint index, GLfloat x);
        ((VertexAttrib1fProc)_glVertexAttrib1f)(index, x);
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
        typedef void (APIENTRY * VertexAttrib1fvProc)(GLuint index, const GLfloat *v);
        ((VertexAttrib1fvProc)_glVertexAttrib1fv)(index, v);
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
        typedef void (APIENTRY * VertexAttrib1sProc)(GLuint index, GLshort x);
        ((VertexAttrib1sProc)_glVertexAttrib1s)(index, x);
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
        typedef void (APIENTRY * VertexAttrib1svProc)(GLuint index, const GLshort *v);
        ((VertexAttrib1svProc)_glVertexAttrib1sv)(index, v);
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
        typedef void (APIENTRY * VertexAttrib2dProc)(GLuint index, GLdouble x, GLdouble y);
        ((VertexAttrib2dProc)_glVertexAttrib2d)(index, x, y);
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
        typedef void (APIENTRY * VertexAttrib2dvProc)(GLuint index, const GLdouble *v);
        ((VertexAttrib2dvProc)_glVertexAttrib2dv)(index, v);
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
        typedef void (APIENTRY * VertexAttrib2fProc)(GLuint index, GLfloat x, GLfloat y);
        ((VertexAttrib2fProc)_glVertexAttrib2f)(index, x, y);
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
        typedef void (APIENTRY * VertexAttrib2fvProc)(GLuint index, const GLfloat *v);
        ((VertexAttrib2fvProc)_glVertexAttrib2fv)(index, v);
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
        typedef void (APIENTRY * VertexAttrib2sProc)(GLuint index, GLshort x, GLshort y);
        ((VertexAttrib2sProc)_glVertexAttrib2s)(index, x, y);
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
        typedef void (APIENTRY * VertexAttrib2svProc)(GLuint index, const GLshort *v);
        ((VertexAttrib2svProc)_glVertexAttrib2sv)(index, v);
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
        typedef void (APIENTRY * VertexAttrib3dProc)(GLuint index, GLdouble x, GLdouble y, GLdouble z);
        ((VertexAttrib3dProc)_glVertexAttrib3d)(index, x, y, z);
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
        typedef void (APIENTRY * VertexAttrib3dvProc)(GLuint index, const GLdouble *v);
        ((VertexAttrib3dvProc)_glVertexAttrib3dv)(index, v);
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
        typedef void (APIENTRY * VertexAttrib3fProc)(GLuint index, GLfloat x, GLfloat y, GLfloat z);
        ((VertexAttrib3fProc)_glVertexAttrib3f)(index, x, y, z);
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
        typedef void (APIENTRY * VertexAttrib3fvProc)(GLuint index, const GLfloat *v);
        ((VertexAttrib3fvProc)_glVertexAttrib3fv)(index, v);
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
        typedef void (APIENTRY * VertexAttrib3sProc)(GLuint index, GLshort x, GLshort y, GLshort z);
        ((VertexAttrib3sProc)_glVertexAttrib3s)(index, x, y, z);
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
        typedef void (APIENTRY * VertexAttrib3svProc)(GLuint index, const GLshort *v);
        ((VertexAttrib3svProc)_glVertexAttrib3sv)(index, v);
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
        typedef void (APIENTRY * VertexAttrib4NbvProc)(GLuint index, const GLbyte *v);
        ((VertexAttrib4NbvProc)_glVertexAttrib4Nbv)(index, v);
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
        typedef void (APIENTRY * VertexAttrib4NivProc)(GLuint index, const GLint *v);
        ((VertexAttrib4NivProc)_glVertexAttrib4Niv)(index, v);
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
        typedef void (APIENTRY * VertexAttrib4NsvProc)(GLuint index, const GLshort *v);
        ((VertexAttrib4NsvProc)_glVertexAttrib4Nsv)(index, v);
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
        typedef void (APIENTRY * VertexAttrib4NubProc)(GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w);
        ((VertexAttrib4NubProc)_glVertexAttrib4Nub)(index, x, y, z, w);
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
        typedef void (APIENTRY * VertexAttrib4NubvProc)(GLuint index, const GLubyte *v);
        ((VertexAttrib4NubvProc)_glVertexAttrib4Nubv)(index, v);
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
        typedef void (APIENTRY * VertexAttrib4NuivProc)(GLuint index, const GLuint *v);
        ((VertexAttrib4NuivProc)_glVertexAttrib4Nuiv)(index, v);
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
        typedef void (APIENTRY * VertexAttrib4NusvProc)(GLuint index, const GLushort *v);
        ((VertexAttrib4NusvProc)_glVertexAttrib4Nusv)(index, v);
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
        typedef void (APIENTRY * VertexAttrib4bvProc)(GLuint index, const GLbyte *v);
        ((VertexAttrib4bvProc)_glVertexAttrib4bv)(index, v);
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
        typedef void (APIENTRY * VertexAttrib4dProc)(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
        ((VertexAttrib4dProc)_glVertexAttrib4d)(index, x, y, z, w);
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
        typedef void (APIENTRY * VertexAttrib4dvProc)(GLuint index, const GLdouble *v);
        ((VertexAttrib4dvProc)_glVertexAttrib4dv)(index, v);
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
        typedef void (APIENTRY * VertexAttrib4fProc)(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
        ((VertexAttrib4fProc)_glVertexAttrib4f)(index, x, y, z, w);
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
        typedef void (APIENTRY * VertexAttrib4fvProc)(GLuint index, const GLfloat *v);
        ((VertexAttrib4fvProc)_glVertexAttrib4fv)(index, v);
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
        typedef void (APIENTRY * VertexAttrib4ivProc)(GLuint index, const GLint *v);
        ((VertexAttrib4ivProc)_glVertexAttrib4iv)(index, v);
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
        typedef void (APIENTRY * VertexAttrib4sProc)(GLuint index, GLshort x, GLshort y, GLshort z, GLshort w);
        ((VertexAttrib4sProc)_glVertexAttrib4s)(index, x, y, z, w);
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
        typedef void (APIENTRY * VertexAttrib4svProc)(GLuint index, const GLshort *v);
        ((VertexAttrib4svProc)_glVertexAttrib4sv)(index, v);
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
        typedef void (APIENTRY * VertexAttrib4ubvProc)(GLuint index, const GLubyte *v);
        ((VertexAttrib4ubvProc)_glVertexAttrib4ubv)(index, v);
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
        typedef void (APIENTRY * VertexAttrib4uivProc)(GLuint index, const GLuint *v);
        ((VertexAttrib4uivProc)_glVertexAttrib4uiv)(index, v);
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
        typedef void (APIENTRY * VertexAttrib4usvProc)(GLuint index, const GLushort *v);
        ((VertexAttrib4usvProc)_glVertexAttrib4usv)(index, v);
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
        typedef void (APIENTRY * VertexAttribPointerProc)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer);
        ((VertexAttribPointerProc)_glVertexAttribPointer)(index, size, type, normalized, stride, pointer);
    }
    else
    {
        NotSupported( "glVertexAttribPointer" );
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
        typedef GLuint (APIENTRY * GetHandleProc) (GLenum pname);
        return ((GetHandleProc)_glGetHandleARB)( GL_PROGRAM_OBJECT_ARB );
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
        if( strLen > 0 ) result = infoLog;
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
        if( strLen > 0 ) result = infoLog;
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
    GLint loc = glGetAttribLocation( program, attribName );
    if( loc < 0 ) return false;

    location = loc;
    return true;
}


///////////////////////////////////////////////////////////////////////////
// static cache of glPrograms flagged for deletion, which will actually
// be deleted in the correct GL context.

typedef std::list<GLuint> GlProgramHandleList;
typedef osg::buffered_object<GlProgramHandleList> DeletedGlProgramCache;

static OpenThreads::Mutex    s_mutex_deletedGlProgramCache;
static DeletedGlProgramCache s_deletedGlProgramCache;

void Program::deleteGlProgram(unsigned int contextID, GLuint program)
{
    if( program )
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(s_mutex_deletedGlProgramCache);

        // add glProgram to the cache for the appropriate context.
        s_deletedGlProgramCache[contextID].push_back(program);
    }
}

void Program::flushDeletedGlPrograms(unsigned int contextID,double /*currentTime*/, double& availableTime)
{
    // if no time available don't try to flush objects.
    if (availableTime<=0.0) return;

    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(s_mutex_deletedGlProgramCache);
    const GL2Extensions* extensions = GL2Extensions::Get(contextID,true);
    if( ! extensions->isGlslSupported() ) return;

    const osg::Timer& timer = *osg::Timer::instance();
    osg::Timer_t start_tick = timer.tick();
    double elapsedTime = 0.0;

    {

        GlProgramHandleList& pList = s_deletedGlProgramCache[contextID];
        for(GlProgramHandleList::iterator titr=pList.begin();
            titr!=pList.end() && elapsedTime<availableTime;
            )
        {
            extensions->glDeleteProgram( *titr );
            titr = pList.erase( titr );
            elapsedTime = timer.delta_s(start_tick,timer.tick());
        }
    }

    availableTime -= elapsedTime;
}


///////////////////////////////////////////////////////////////////////////
// osg::Program
///////////////////////////////////////////////////////////////////////////

Program::Program()
{
}


Program::Program(const Program& rhs, const osg::CopyOp& copyop):
    osg::StateAttribute(rhs, copyop)
{
    osg::notify(osg::FATAL) << "how got here?" << std::endl;
}


Program::~Program()
{
    // inform any attached Shaders that we're going away
    for( unsigned int i=0; i < _shaderList.size(); ++i )
    {
        _shaderList[i]->removeProgramRef( this );
    }
}


int Program::compare(const osg::StateAttribute& sa) const
{
    // check the types are equal and then create the rhs variable
    // used by the COMPARE_StateAttribute_Paramter macro's below.
    COMPARE_StateAttribute_Types(Program,sa)
    
    if( _shaderList.size() < rhs._shaderList.size() ) return -1;
    if( rhs._shaderList.size() < _shaderList.size() ) return 1;

    if( getName() < rhs.getName() ) return -1;
    if( rhs.getName() < getName() ) return 1;

    ShaderList::const_iterator litr=_shaderList.begin();
    ShaderList::const_iterator ritr=rhs._shaderList.begin();
    for(;
        litr!=_shaderList.end();
        ++litr,++ritr)
    {
        int result = (*litr)->compare(*(*ritr));
        if (result!=0) return result;
    }

    return 0; // passed all the above comparison macro's, must be equal.
}


void Program::compileGLObjects( osg::State& state ) const
{
    if( isFixedFunction() ) return;

    const unsigned int contextID = state.getContextID();

    for( unsigned int i=0; i < _shaderList.size(); ++i )
    {
        _shaderList[i]->compileShader( contextID );
    }

    getPCP( contextID )->linkProgram();
}


void Program::dirtyProgram()
{
    // mark our PCPs as needing relink
    for( unsigned int cxt=0; cxt < _pcpList.size(); ++cxt )
    {
        if( _pcpList[cxt].valid() ) _pcpList[cxt]->requestLink();
    }
}


void Program::releaseGLObjects(osg::State* state) const
{
    for( unsigned int i=0; i < _shaderList.size(); ++i )
    {
        if (_shaderList[i].valid()) _shaderList[i]->releaseGLObjects(state);
    }

    if (!state) _pcpList.setAllElementsTo(0);
    else
    {
        unsigned int contextID = state->getContextID();
        _pcpList[contextID] = 0;
    }   
}


bool Program::addShader( Shader* shader )
{
    if( !shader ) return false;

    // Shader can only be added once to a Program
    for( unsigned int i=0; i < _shaderList.size(); ++i )
    {
        if( shader == _shaderList[i].get() ) return false;
    }

    shader->addProgramRef( this );
    _shaderList.push_back( shader );
    dirtyProgram();
    return true;
}


bool Program::removeShader( Shader* shader )
{
    if( !shader ) return false;

    // Shader must exist to be removed.
    for( ShaderList::iterator itr = _shaderList.begin();
         itr != _shaderList.end();
         ++itr)
    {
        if( shader == itr->get() )
        {
            shader->removeProgramRef( this );
            _shaderList.erase(itr);
            dirtyProgram();
            return true;
        }
    }
    return false;
}


void Program::addBindAttribLocation( const std::string& name, GLuint index )
{
    _attribBindingList[name] = index;
    dirtyProgram();
}

void Program::removeBindAttribLocation( const std::string& name )
{
    _attribBindingList.erase(name);
    dirtyProgram();
}


void Program::apply( osg::State& state ) const
{
    const unsigned int contextID = state.getContextID();
    const GL2Extensions* extensions = GL2Extensions::Get(contextID,true);
    if( ! extensions->isGlslSupported() ) return;

    if( isFixedFunction() )
    {
        extensions->glUseProgram( 0 );
        state.setLastAppliedProgramObject(0);
        return;
    }

    PerContextProgram* pcp = getPCP( contextID );
    if( pcp->needsLink() ) compileGLObjects( state );
    if( pcp->isLinked() )
    {
        // for shader debugging: to minimize performance impact,
        // optionally validate based on notify level.
        // TODO: enable this using notify level, or perhaps its own getenv()?
        if( osg::isNotifyEnabled(osg::INFO) )
            pcp->validateProgram();

        pcp->useProgram();
        state.setLastAppliedProgramObject(pcp);
    }
    else
    {
        // program not usable, fallback to fixed function.
        extensions->glUseProgram( 0 );
        state.setLastAppliedProgramObject(0);
    }
}


Program::PerContextProgram* Program::getPCP(unsigned int contextID) const
{
    if( ! _pcpList[contextID].valid() )
    {
        _pcpList[contextID] = new PerContextProgram( this, contextID );

        // attach all PCSs to this new PCP
        for( unsigned int i=0; i < _shaderList.size(); ++i )
        {
            _shaderList[i]->attachShader( contextID, _pcpList[contextID]->getHandle() );
        }
    }
    return _pcpList[contextID].get();
}


bool Program::isFixedFunction() const
{
    // A Program object having no attached Shaders is a special case:
    // it indicates that programmable shading is to be disabled,
    // and thus use GL 1.x "fixed functionality" rendering.
    return _shaderList.empty();
}


bool Program::getGlProgramInfoLog(unsigned int contextID, std::string& log) const
{
    return getPCP( contextID )->getInfoLog( log );
}

const Program::ActiveVarInfoMap& Program::getActiveUniforms(unsigned int contextID) const
{
    return getPCP( contextID )->getActiveUniforms();
}

const Program::ActiveVarInfoMap& Program::getActiveAttribs(unsigned int contextID) const
{
    return getPCP( contextID )->getActiveAttribs();
}

///////////////////////////////////////////////////////////////////////////
// osg::Program::PerContextProgram
// PCP is an OSG abstraction of the per-context glProgram
///////////////////////////////////////////////////////////////////////////

Program::PerContextProgram::PerContextProgram(const Program* program, unsigned int contextID ) :
        osg::Referenced(),
        _contextID( contextID )
{
    _program = program;
    _extensions = GL2Extensions::Get( _contextID, true );
    _glProgramHandle = _extensions->glCreateProgram();
    requestLink();
}

Program::PerContextProgram::~PerContextProgram()
{
    Program::deleteGlProgram( _contextID, _glProgramHandle );
}


void Program::PerContextProgram::requestLink()
{
    _needsLink = true;
    _isLinked = false;
}


void Program::PerContextProgram::linkProgram()
{
    if( ! _needsLink ) return;
    _needsLink = false;

    osg::notify(osg::INFO)
        << "Linking osg::Program \"" << _program->getName() << "\""
        << " id=" << _glProgramHandle
        << " contextID=" << _contextID
        <<  std::endl;

    _uniformInfoMap.clear();
    _attribInfoMap.clear();
    _lastAppliedUniformList.clear();

    // set any explicit vertex attribute bindings
    const AttribBindingList& bindlist = _program->getAttribBindingList();
    for( AttribBindingList::const_iterator itr = bindlist.begin();
        itr != bindlist.end(); ++itr )
    {
        _extensions->glBindAttribLocation( _glProgramHandle, itr->second, itr->first.c_str() );
    }

    // link the glProgram
    GLint linked = GL_FALSE;
    _extensions->glLinkProgram( _glProgramHandle );
    _extensions->glGetProgramiv( _glProgramHandle, GL_LINK_STATUS, &linked );
    _isLinked = (linked == GL_TRUE);
    if( ! _isLinked )
    {
        osg::notify(osg::WARN) << "glLinkProgram \""<< _program->getName() << "\" FAILED" << std::endl;

        std::string infoLog;
        if( getInfoLog(infoLog) )
        {
            osg::notify(osg::WARN) << "Program \""<< _program->getName() << "\" " 
                                      "infolog:\n" << infoLog << std::endl;
        }
        
        return;
    }
    else
    {
        std::string infoLog;
        if( getInfoLog(infoLog) )
        {
            osg::notify(osg::INFO) << "Program \""<< _program->getName() << "\" "<<
                                      "link succeded, infolog:\n" << infoLog << std::endl;
        }
    }

    // build _uniformInfoMap
    GLint numUniforms = 0;
    GLsizei maxLen = 0;
    _extensions->glGetProgramiv( _glProgramHandle, GL_ACTIVE_UNIFORMS, &numUniforms );
    _extensions->glGetProgramiv( _glProgramHandle, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxLen );
    if( (numUniforms > 0) && (maxLen > 1) )
    {
        GLint size = 0;
        GLenum type = 0;
        GLchar* name = new GLchar[maxLen];

        for( GLint i = 0; i < numUniforms; ++i )
        {
            _extensions->glGetActiveUniform( _glProgramHandle,
                    i, maxLen, 0, &size, &type, name );

            GLint loc = _extensions->glGetUniformLocation( _glProgramHandle, name );
            
            if( loc != -1 )
            {
                _uniformInfoMap[name] = ActiveVarInfo(loc,type,size);

                osg::notify(osg::INFO)
                    << "\tUniform \"" << name << "\""
                    << " loc="<< loc
                    << " size="<< size
                    << " type=" << Uniform::getTypename((Uniform::Type)type)
                    << std::endl;
            }
        }
        delete [] name;
    }

    // build _attribInfoMap
    GLint numAttrib = 0;
    _extensions->glGetProgramiv( _glProgramHandle, GL_ACTIVE_ATTRIBUTES, &numAttrib );
    _extensions->glGetProgramiv( _glProgramHandle, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &maxLen );
    if( (numAttrib > 0) && (maxLen > 1) )
    {
        GLint size = 0;
        GLenum type = 0;
        GLchar* name = new GLchar[maxLen];

        for( GLint i = 0; i < numAttrib; ++i )
        {
            _extensions->glGetActiveAttrib( _glProgramHandle,
                    i, maxLen, 0, &size, &type, name );

            GLint loc = _extensions->glGetAttribLocation( _glProgramHandle, name );
            
            if( loc != -1 )
            {
                _attribInfoMap[name] = ActiveVarInfo(loc,type,size);

                osg::notify(osg::INFO)
                    << "\tAttrib \"" << name << "\""
                    << " loc=" << loc
                    << " size=" << size
                    << std::endl;
            }
        }
        delete [] name;
    }
    osg::notify(osg::INFO) << std::endl;
}

bool Program::PerContextProgram::validateProgram()
{
    GLint validated = GL_FALSE;
    _extensions->glValidateProgram( _glProgramHandle );
    _extensions->glGetProgramiv( _glProgramHandle, GL_VALIDATE_STATUS, &validated );
    if( validated == GL_TRUE)
        return true;

    osg::notify(osg::INFO)
        << "glValidateProgram FAILED \"" << _program->getName() << "\""
        << " id=" << _glProgramHandle
        << " contextID=" << _contextID
        <<  std::endl;

    std::string infoLog;
    if( getInfoLog(infoLog) )
        osg::notify(osg::INFO) << "infolog:\n" << infoLog << std::endl;

    osg::notify(osg::INFO) << std::endl;
    
    return false;
}

bool Program::PerContextProgram::getInfoLog( std::string& infoLog ) const
{
    return _extensions->getProgramInfoLog( _glProgramHandle, infoLog );
}

void Program::PerContextProgram::useProgram() const
{
    _extensions->glUseProgram( _glProgramHandle  );
}


/*EOF*/
