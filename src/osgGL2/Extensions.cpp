/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2003 Robert Osfield 
 * Copyright (C) 2003 3Dlabs Inc. Ltd.
 *
 * This application is open source and may be redistributed and/or modified   
 * freely and without restriction, both in commericial and non commericial
 * applications, as long as this copyright notice is maintained.
 * 
 * This application is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

/* file:	src/osgGL2/Extensions.cpp
 * author:	Mike Weiblen 2003-09-12
 *
 * See http://www.3dlabs.com/opengl2/ for more information regarding
 * the OpenGL Shading Language.
*/


#include <osg/Notify>
#include <osg/GLExtensions>
#include <osg/buffered_value>
#include <osg/ref_ptr>

#include <osgGL2/Extensions>

using namespace osgGL2;


Extensions::Extensions()
{
    setupGLExtensions();
}

Extensions::Extensions(const Extensions& rhs) : osg::Referenced()
{
    _isShaderObjectsSupported = rhs._isShaderObjectsSupported;
    _isVertexShaderSupported = rhs._isVertexShaderSupported;
    _isFragmentShaderSupported = rhs._isFragmentShaderSupported;

    _glCreateShaderObject = rhs._glCreateShaderObject;
    _glCreateProgramObject = rhs._glCreateProgramObject;
    _glDeleteObject = rhs._glDeleteObject;
    _glAttachObject = rhs._glAttachObject;
    _glDetachObject = rhs._glDetachObject;
    _glShaderSource = rhs._glShaderSource;
    _glCompileShader = rhs._glCompileShader;
    _glBindAttribLocation = rhs._glBindAttribLocation;
    _glLinkProgram = rhs._glLinkProgram;
    _glUseProgramObject = rhs._glUseProgramObject;
    _glGetInfoLog = rhs._glGetInfoLog;
    _glGetAttachedObjects = rhs._glGetAttachedObjects;
    _glGetShaderSource = rhs._glGetShaderSource;
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
    _glGetUniformLocation = rhs._glGetUniformLocation;
    _glGetAttribLocation = rhs._glGetAttribLocation;
    _glGetActiveUniform = rhs._glGetActiveUniform;
    _glGetActiveAttribs = rhs._glGetActiveAttribs;
    _glGetUniformfv = rhs._glGetUniformfv;
    _glGetUniformiv = rhs._glGetUniformiv;
    _glGetObjectParameterfv = rhs._glGetObjectParameterfv;
    _glGetObjectParameteriv = rhs._glGetObjectParameteriv;
    _glGetHandle = rhs._glGetHandle;
}


void Extensions::lowestCommonDenominator(const Extensions& rhs)
{
    if (!rhs._isShaderObjectsSupported) _isShaderObjectsSupported = false;
    if (!rhs._isVertexShaderSupported) _isVertexShaderSupported = false;
    if (!rhs._isFragmentShaderSupported) _isFragmentShaderSupported = false;

    if (!rhs._glCreateShaderObject) _glCreateShaderObject = 0;
    if (!rhs._glCreateProgramObject) _glCreateProgramObject = 0;
    if (!rhs._glDeleteObject) _glDeleteObject = 0;
    if (!rhs._glAttachObject) _glAttachObject = 0;
    if (!rhs._glDetachObject) _glDetachObject = 0;
    if (!rhs._glShaderSource) _glShaderSource = 0;
    if (!rhs._glCompileShader) _glCompileShader = 0;
    if (!rhs._glBindAttribLocation) _glBindAttribLocation = 0;
    if (!rhs._glLinkProgram) _glLinkProgram = 0;
    if (!rhs._glUseProgramObject) _glUseProgramObject = 0;
    if (!rhs._glGetInfoLog) _glGetInfoLog = 0;
    if (!rhs._glGetAttachedObjects) _glGetAttachedObjects = 0;
    if (!rhs._glGetShaderSource) _glGetShaderSource = 0;
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
    if (!rhs._glGetUniformLocation) _glGetUniformLocation = 0;
    if (!rhs._glGetAttribLocation) _glGetAttribLocation = 0;
    if (!rhs._glGetActiveUniform) _glGetActiveUniform = 0;
    if (!rhs._glGetActiveAttribs) _glGetActiveAttribs = 0;
    if (!rhs._glGetUniformfv) _glGetUniformfv = 0;
    if (!rhs._glGetUniformiv) _glGetUniformiv = 0;
    if (!rhs._glGetObjectParameterfv) _glGetObjectParameterfv = 0;
    if (!rhs._glGetObjectParameteriv) _glGetObjectParameteriv = 0;
    if (!rhs._glGetHandle) _glGetHandle = 0;
}

void Extensions::setupGLExtensions()
{
    _isShaderObjectsSupported = osg::isGLExtensionSupported("GL_ARB_shader_objects");
    _isVertexShaderSupported = osg::isGLExtensionSupported("GL_ARB_vertex_shader");
    _isFragmentShaderSupported = osg::isGLExtensionSupported("GL_ARB_fragment_shader");

    _glCreateShaderObject = osg::getGLExtensionFuncPtr("glCreateShaderObjectARB");
    _glCreateProgramObject = osg::getGLExtensionFuncPtr("glCreateProgramObjectARB");
    _glDeleteObject = osg::getGLExtensionFuncPtr("glDeleteObjectARB");
    _glAttachObject = osg::getGLExtensionFuncPtr("glAttachObjectARB");
    _glDetachObject = osg::getGLExtensionFuncPtr("glDetachObjectARB");
    _glShaderSource = osg::getGLExtensionFuncPtr("glShaderSourceARB");
    _glCompileShader = osg::getGLExtensionFuncPtr("glCompileShaderARB");
    _glBindAttribLocation = osg::getGLExtensionFuncPtr("glBindAttribLocationARB");
    _glLinkProgram = osg::getGLExtensionFuncPtr("glLinkProgramARB");
    _glUseProgramObject = osg::getGLExtensionFuncPtr("glUseProgramObjectARB");
    _glGetInfoLog = osg::getGLExtensionFuncPtr("glGetInfoLogARB");
    _glGetAttachedObjects = osg::getGLExtensionFuncPtr("glGetAttachedObjectsARB");
    _glGetShaderSource = osg::getGLExtensionFuncPtr("glGetShaderSourceARB");
    _glUniform1f = osg::getGLExtensionFuncPtr("glUniform1fARB");
    _glUniform2f = osg::getGLExtensionFuncPtr("glUniform2fARB");
    _glUniform3f = osg::getGLExtensionFuncPtr("glUniform3fARB");
    _glUniform4f = osg::getGLExtensionFuncPtr("glUniform4fARB");
    _glUniform1i = osg::getGLExtensionFuncPtr("glUniform1iARB");
    _glUniform2i = osg::getGLExtensionFuncPtr("glUniform2iARB");
    _glUniform3i = osg::getGLExtensionFuncPtr("glUniform3iARB");
    _glUniform4i = osg::getGLExtensionFuncPtr("glUniform4iARB");
    _glUniform1fv = osg::getGLExtensionFuncPtr("glUniform1fvARB");
    _glUniform2fv = osg::getGLExtensionFuncPtr("glUniform2fvARB");
    _glUniform3fv = osg::getGLExtensionFuncPtr("glUniform3fvARB");
    _glUniform4fv = osg::getGLExtensionFuncPtr("glUniform4fvARB");
    _glUniform1iv = osg::getGLExtensionFuncPtr("glUniform1ivARB");
    _glUniform2iv = osg::getGLExtensionFuncPtr("glUniform2ivARB");
    _glUniform3iv = osg::getGLExtensionFuncPtr("glUniform3ivARB");
    _glUniform4iv = osg::getGLExtensionFuncPtr("glUniform4ivARB");
    _glUniformMatrix2fv = osg::getGLExtensionFuncPtr("glUniformMatrix2fvARB");
    _glUniformMatrix3fv = osg::getGLExtensionFuncPtr("glUniformMatrix3fvARB");
    _glUniformMatrix4fv = osg::getGLExtensionFuncPtr("glUniformMatrix4fvARB");
    _glGetUniformLocation = osg::getGLExtensionFuncPtr("glGetUniformLocationARB");
    _glGetAttribLocation = osg::getGLExtensionFuncPtr("glGetAttribLocationARB");
    _glGetActiveUniform = osg::getGLExtensionFuncPtr("glGetActiveUniformARB");
    _glGetActiveAttribs = osg::getGLExtensionFuncPtr("glGetActiveAttribsARB");
    _glGetUniformfv = osg::getGLExtensionFuncPtr("glGetUniformfvARB");
    _glGetUniformiv = osg::getGLExtensionFuncPtr("glGetUniformivARB");
    _glGetObjectParameterfv = osg::getGLExtensionFuncPtr("glGetObjectParameterfvARB");
    _glGetObjectParameteriv = osg::getGLExtensionFuncPtr("glGetObjectParameterivARB");
    _glGetHandle = osg::getGLExtensionFuncPtr("glGetHandleARB");
}

/***************************************************************************/
// Static array of per-context osgGL2::Extensions instances

typedef osg::buffered_value< osg::ref_ptr<Extensions> > BufferedExtensions;
static BufferedExtensions s_extensions;

Extensions* Extensions::Get(unsigned int contextID, bool createIfNotInitalized)
{
    if (!s_extensions[contextID] && createIfNotInitalized)
	    s_extensions[contextID] = new Extensions;

    return s_extensions[contextID].get();
}

void Extensions::Set(unsigned int contextID, Extensions* extensions)
{
    s_extensions[contextID] = extensions;
}

/***************************************************************************/

GLhandleARB Extensions::glCreateShaderObject(GLenum shaderType) const
{
    if (_glCreateShaderObject)
    {
	typedef GLhandleARB (APIENTRY * CreateShaderObjectProc) (GLenum shaderType);
	return ((CreateShaderObjectProc)_glCreateShaderObject)(shaderType);
    }
    else
    {
	osg::notify(osg::WARN)<<"Error: glCreateShaderObject not supported by OpenGL driver"<<std::endl;
	return -1;	//TODO
    }
}

GLhandleARB Extensions::glCreateProgramObject() const
{
    if (_glCreateProgramObject)
    {
	typedef GLhandleARB (APIENTRY * CreateProgramObjectProc) ();
	return ((CreateProgramObjectProc)_glCreateProgramObject)();
    }
    else
    {
	osg::notify(osg::WARN)<<"Error: glCreateProgramObject not supported by OpenGL driver"<<std::endl;
	return -1;	//TODO
    }
}

void Extensions::glDeleteObject(GLhandleARB obj) const
{
    if (_glDeleteObject)
    {
	typedef void (APIENTRY * DeleteObjectProc) (GLhandleARB obj);
	((DeleteObjectProc)_glDeleteObject)(obj);
    }
    else
    {
	osg::notify(osg::WARN)<<"Error: glDeleteObject not supported by OpenGL driver"<<std::endl;
    }
}

void Extensions::glAttachObject(GLhandleARB containerObj, GLhandleARB obj) const
{
    if (_glAttachObject)
    {
	typedef void (APIENTRY * AttachObjectProc) (GLhandleARB containerObj, GLhandleARB obj);
	((AttachObjectProc)_glAttachObject)(containerObj, obj);
    }
    else
    {
	osg::notify(osg::WARN)<<"Error: glAttachObject not supported by OpenGL driver"<<std::endl;
    }
}

void Extensions::glDetachObject(GLhandleARB containerObj, GLhandleARB attachedObj) const
{
    if (_glDetachObject)
    {
	typedef void (APIENTRY * DetachObjectProc) (GLhandleARB containerObj, GLhandleARB attachedObj);
	((DetachObjectProc)_glDetachObject)(containerObj, attachedObj);
    }
    else
    {
	osg::notify(osg::WARN)<<"Error: glDetachObject not supported by OpenGL driver"<<std::endl;
    }
}

void Extensions::glShaderSource(GLhandleARB shaderObj, GLsizei count, const GLcharARB **strings, const GLint *length) const
{
    if (_glShaderSource)
    {
	typedef void (APIENTRY * ShaderSourceProc) (GLhandleARB shaderObj, GLsizei count, const GLcharARB **strings, const GLint *length);
	((ShaderSourceProc)_glShaderSource)(shaderObj, count, strings, length);
    }
    else
    {
	osg::notify(osg::WARN)<<"Error: glShaderSource not supported by OpenGL driver"<<std::endl;
    }
}

void Extensions::glCompileShader(GLhandleARB shaderObj) const
{
    if (_glCompileShader)
    {
	typedef void (APIENTRY * CompileShaderProc) (GLhandleARB shaderObj);
	((CompileShaderProc)_glCompileShader)(shaderObj);
    }
    else
    {
	osg::notify(osg::WARN)<<"Error: glCompileShader not supported by OpenGL driver"<<std::endl;
    }
}

void Extensions::glBindAttribLocation(GLhandleARB programObj, GLuint index, const GLcharARB *name) const
{
    if (_glBindAttribLocation)
    {
	typedef void (APIENTRY * BindAttribLocationProc) (GLhandleARB programObj, GLuint index, const GLcharARB *name);
	((BindAttribLocationProc)_glBindAttribLocation)(programObj, index, name);
    }
    else
    {
	osg::notify(osg::WARN)<<"Error: glBindAttribLocation not supported by OpenGL driver"<<std::endl;
    }
}

void Extensions::glLinkProgram(GLhandleARB programObj) const
{
    if (_glLinkProgram)
    {
	typedef void (APIENTRY * LinkProgramProc) (GLhandleARB programObj);
	((LinkProgramProc)_glLinkProgram)(programObj);
    }
    else
    {
	osg::notify(osg::WARN)<<"Error: glLinkProgram not supported by OpenGL driver"<<std::endl;
    }
}

void Extensions::glUseProgramObject(GLhandleARB programObj) const
{
    if (_glUseProgramObject)
    {
	typedef void (APIENTRY * UseProgramObjectProc) (GLhandleARB programObj);
	((UseProgramObjectProc)_glUseProgramObject)(programObj);
    }
    else
    {
	osg::notify(osg::WARN)<<"Error: glUseProgramObject not supported by OpenGL driver"<<std::endl;
    }
}

void Extensions::glGetInfoLog(GLhandleARB obj,GLsizei maxLength, GLsizei *length, GLcharARB *infoLog) const
{
    if (_glGetInfoLog)
    {
	typedef void (APIENTRY * GetInfoLogProc) (GLhandleARB obj,GLsizei maxLength, GLsizei *length, GLcharARB *infoLog);
	((GetInfoLogProc)_glGetInfoLog)(obj,maxLength, length, infoLog);
    }
    else
    {
	osg::notify(osg::WARN)<<"Error: glGetInfoLog not supported by OpenGL driver"<<std::endl;
    }
}

void Extensions::glGetAttachedObjects(GLhandleARB containerObj, GLsizei maxCount, GLsizei *count, GLhandleARB *obj) const
{
    if (_glGetAttachedObjects)
    {
	typedef void (APIENTRY * GetAttachedObjectsProc) (GLhandleARB containerObj, GLsizei maxCount, GLsizei *count, GLhandleARB *obj);
	((GetAttachedObjectsProc)_glGetAttachedObjects)(containerObj, maxCount, count, obj);
    }
    else
    {
	osg::notify(osg::WARN)<<"Error: glGetAttachedObjects not supported by OpenGL driver"<<std::endl;
    }
}

void Extensions::glGetShaderSource(GLhandleARB obj, GLsizei maxLength, GLsizei *length, GLcharARB *source) const
{
    if (_glGetShaderSource)
    {
	typedef void (APIENTRY * GetShaderSourceProc) (GLhandleARB obj, GLsizei maxLength, GLsizei *length, GLcharARB *source);
	((GetShaderSourceProc)_glGetShaderSource)(obj, maxLength, length, source);
    }
    else
    {
	osg::notify(osg::WARN)<<"Error: glGetShaderSource not supported by OpenGL driver"<<std::endl;
    }
}

void Extensions::glUniform1f(GLint location, GLfloat v0) const
{
    if (_glUniform1f)
    {
	typedef void (APIENTRY * Uniform1fProc) (GLint location, GLfloat v0);
	((Uniform1fProc)_glUniform1f)(location, v0);
    }
    else
    {
	osg::notify(osg::WARN)<<"Error: glUniform1f not supported by OpenGL driver"<<std::endl;
    }
}

void Extensions::glUniform2f(GLint location, GLfloat v0, GLfloat v1) const
{
    if (_glUniform2f)
    {
	typedef void (APIENTRY * Uniform2fProc) (GLint location, GLfloat v0, GLfloat v1);
	((Uniform2fProc)_glUniform2f)(location, v0, v1);
    }
    else
    {
	osg::notify(osg::WARN)<<"Error: glUniform2f not supported by OpenGL driver"<<std::endl;
    }
}

void Extensions::glUniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2) const
{
    if (_glUniform3f)
    {
	typedef void (APIENTRY * Uniform3fProc) (GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
	((Uniform3fProc)_glUniform3f)(location, v0, v1, v2);
    }
    else
    {
	osg::notify(osg::WARN)<<"Error: glUniform3f not supported by OpenGL driver"<<std::endl;
    }
}

void Extensions::glUniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) const
{
    if (_glUniform4f)
    {
	typedef void (APIENTRY * Uniform4fProc) (GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
	((Uniform4fProc)_glUniform4f)(location, v0, v1, v2, v3);
    }
    else
    {
	osg::notify(osg::WARN)<<"Error: glUniform4f not supported by OpenGL driver"<<std::endl;
    }
}

void Extensions::glUniform1i(GLint location, GLint v0) const
{
    if (_glUniform1i)
    {
	typedef void (APIENTRY * Uniform1iProc) (GLint location, GLint v0);
	((Uniform1iProc)_glUniform1i)(location, v0);
    }
    else
    {
	osg::notify(osg::WARN)<<"Error: glUniform1i not supported by OpenGL driver"<<std::endl;
    }
}

void Extensions::glUniform2i(GLint location, GLint v0, GLint v1) const
{
    if (_glUniform2i)
    {
	typedef void (APIENTRY * Uniform2iProc) (GLint location, GLint v0, GLint v1);
	((Uniform2iProc)_glUniform2i)(location, v0, v1);
    }
    else
    {
	osg::notify(osg::WARN)<<"Error: glUniform2i not supported by OpenGL driver"<<std::endl;
    }
}

void Extensions::glUniform3i(GLint location, GLint v0, GLint v1, GLint v2) const
{
    if (_glUniform3i)
    {
	typedef void (APIENTRY * Uniform3iProc) (GLint location, GLint v0, GLint v1, GLint v2);
	((Uniform3iProc)_glUniform3i)(location, v0, v1, v2);
    }
    else
    {
	osg::notify(osg::WARN)<<"Error: glUniform3i not supported by OpenGL driver"<<std::endl;
    }
}

void Extensions::glUniform4i(GLint location, GLint v0, GLint v1, GLint v2, GLint v3) const
{
    if (_glUniform4i)
    {
	typedef void (APIENTRY * Uniform4iProc) (GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
	((Uniform4iProc)_glUniform4i)(location, v0, v1, v2, v3);
    }
    else
    {
	osg::notify(osg::WARN)<<"Error: glUniform4i not supported by OpenGL driver"<<std::endl;
    }
}

void Extensions::glUniform1fv(GLint location, GLsizei count, const GLfloat *value) const
{
    if (_glUniform1fv)
    {
	typedef void (APIENTRY * Uniform1fvProc) (GLint location, GLsizei count, const GLfloat *value);
	((Uniform1fvProc)_glUniform1fv)(location, count, value);
    }
    else
    {
	osg::notify(osg::WARN)<<"Error: glUniform1fv not supported by OpenGL driver"<<std::endl;
    }
}

void Extensions::glUniform2fv(GLint location, GLsizei count, const GLfloat *value) const
{
    if (_glUniform2fv)
    {
	typedef void (APIENTRY * Uniform2fvProc) (GLint location, GLsizei count, const GLfloat *value);
	((Uniform2fvProc)_glUniform2fv)(location, count, value);
    }
    else
    {
	osg::notify(osg::WARN)<<"Error: glUniform2fv not supported by OpenGL driver"<<std::endl;
    }
}

void Extensions::glUniform3fv(GLint location, GLsizei count, const GLfloat *value) const
{
    if (_glUniform3fv)
    {
	typedef void (APIENTRY * Uniform3fvProc) (GLint location, GLsizei count, const GLfloat *value);
	((Uniform3fvProc)_glUniform3fv)(location, count, value);
    }
    else
    {
	osg::notify(osg::WARN)<<"Error: glUniform3fv not supported by OpenGL driver"<<std::endl;
    }
}

void Extensions::glUniform4fv(GLint location, GLsizei count, const GLfloat *value) const
{
    if (_glUniform4fv)
    {
	typedef void (APIENTRY * Uniform4fvProc) (GLint location, GLsizei count, const GLfloat *value);
	((Uniform4fvProc)_glUniform4fv)(location, count, value);
    }
    else
    {
	osg::notify(osg::WARN)<<"Error: glUniform4fv not supported by OpenGL driver"<<std::endl;
    }
}

void Extensions::glUniform1iv(GLint location, GLsizei count, const GLint *value) const
{
    if (_glUniform1iv)
    {
	typedef void (APIENTRY * Uniform1ivProc) (GLint location, GLsizei count, const GLint *value);
	((Uniform1ivProc)_glUniform1iv)(location, count, value);
    }
    else
    {
	osg::notify(osg::WARN)<<"Error: glUniform1iv not supported by OpenGL driver"<<std::endl;
    }
}

void Extensions::glUniform2iv(GLint location, GLsizei count, const GLint *value) const
{
    if (_glUniform2iv)
    {
	typedef void (APIENTRY * Uniform2ivProc) (GLint location, GLsizei count, const GLint *value);
	((Uniform2ivProc)_glUniform2iv)(location, count, value);
    }
    else
    {
	osg::notify(osg::WARN)<<"Error: glUniform2iv not supported by OpenGL driver"<<std::endl;
    }
}

void Extensions::glUniform3iv(GLint location, GLsizei count, const GLint *value) const
{
    if (_glUniform3iv)
    {
	typedef void (APIENTRY * Uniform3ivProc) (GLint location, GLsizei count, const GLint *value);
	((Uniform3ivProc)_glUniform3iv)(location, count, value);
    }
    else
    {
	osg::notify(osg::WARN)<<"Error: glUniform3iv not supported by OpenGL driver"<<std::endl;
    }
}

void Extensions::glUniform4iv(GLint location, GLsizei count, const GLint *value) const
{
    if (_glUniform4iv)
    {
	typedef void (APIENTRY * Uniform4ivProc) (GLint location, GLsizei count, const GLint *value);
	((Uniform4ivProc)_glUniform4iv)(location, count, value);
    }
    else
    {
	osg::notify(osg::WARN)<<"Error: glUniform4iv not supported by OpenGL driver"<<std::endl;
    }
}

void Extensions::glUniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) const
{
    if (_glUniformMatrix2fv)
    {
	typedef void (APIENTRY * UniformMatrix2fvProc) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
	((UniformMatrix2fvProc)_glUniformMatrix2fv)(location, count, transpose, value);
    }
    else
    {
	osg::notify(osg::WARN)<<"Error: glUniformMatrix2fv not supported by OpenGL driver"<<std::endl;
    }
}

void Extensions::glUniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) const
{
    if (_glUniformMatrix3fv)
    {
	typedef void (APIENTRY * UniformMatrix3fvProc) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
	((UniformMatrix3fvProc)_glUniformMatrix3fv)(location, count, transpose, value);
    }
    else
    {
	osg::notify(osg::WARN)<<"Error: glUniformMatrix3fv not supported by OpenGL driver"<<std::endl;
    }
}

void Extensions::glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) const
{
    if (_glUniformMatrix4fv)
    {
	typedef void (APIENTRY * UniformMatrix4fvProc) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
	((UniformMatrix4fvProc)_glUniformMatrix4fv)(location, count, transpose, value);
    }
    else
    {
	osg::notify(osg::WARN)<<"Error: glUniformMatrix4fv not supported by OpenGL driver"<<std::endl;
    }
}

GLint Extensions::glGetUniformLocation(GLhandleARB programObject, const GLcharARB *name) const
{
    if (_glGetUniformLocation)
    {
	typedef GLint (APIENTRY * GetUniformLocationProc) (GLhandleARB programObject, const GLcharARB *name);
	return ((GetUniformLocationProc)_glGetUniformLocation)(programObject, name);
    }
    else
    {
	osg::notify(osg::WARN)<<"Error: glGetUniformLocation not supported by OpenGL driver"<<std::endl;
	return -1;	//TODO
    }
}

GLint Extensions::glGetAttribLocation(GLhandleARB programObj, const GLcharARB *name) const
{
    if (_glGetAttribLocation)
    {
	typedef GLint (APIENTRY * GetAttribLocationProc) (GLhandleARB programObj, const GLcharARB *name);
	return ((GetAttribLocationProc)_glGetAttribLocation)(programObj, name);
    }
    else
    {
	osg::notify(osg::WARN)<<"Error: glGetAttribLocation not supported by OpenGL driver"<<std::endl;
	return -1;	//TODO
    }
}

void Extensions::glGetActiveUniform(GLhandleARB programObj, GLuint index, GLsizei maxLength, GLsizei *length, GLint *size, GLint *type, GLcharARB *name) const
{
    if (_glGetActiveUniform)
    {
	typedef void (APIENTRY * GetActiveUniformProc) (GLhandleARB programObj, GLuint index, GLsizei maxLength, GLsizei *length, GLint *size, GLint *type, GLcharARB *name);
	((GetActiveUniformProc)_glGetActiveUniform)(programObj, index, maxLength, length, size, type, name);
    }
    else
    {
	osg::notify(osg::WARN)<<"Error: glGetActiveUniform not supported by OpenGL driver"<<std::endl;
    }
}

void Extensions::glGetActiveAttribs(GLhandleARB programObj, GLint *count, const GLint **size, const GLenum **type, const GLcharARB* const **attributes) const
{
    if (_glGetActiveAttribs)
    {
	typedef void (APIENTRY * GetActiveAttribsProc) (GLhandleARB programObj, GLint *count, const GLint **size, const GLenum **type, const GLcharARB* const **attributes);
	((GetActiveAttribsProc)_glGetActiveAttribs)(programObj, count, size, type, attributes);
    }
    else
    {
	osg::notify(osg::WARN)<<"Error: glGetActiveAttribs not supported by OpenGL driver"<<std::endl;
    }
}

void Extensions::glGetUniformfv(GLhandleARB programObj, GLint location, GLfloat *params) const
{
    if (_glGetUniformfv)
    {
	typedef void (APIENTRY * GetUniformfvProc) (GLhandleARB programObj, GLint location, GLfloat *params);
	((GetUniformfvProc)_glGetUniformfv)(programObj, location, params);
    }
    else
    {
	osg::notify(osg::WARN)<<"Error: glGetUniformfv not supported by OpenGL driver"<<std::endl;
    }
}

void Extensions::glGetUniformiv(GLhandleARB programObj, GLint location, GLint *params) const
{
    if (_glGetUniformiv)
    {
	typedef void (APIENTRY * GetUniformivProc) (GLhandleARB programObj, GLint location, GLint *params);
	((GetUniformivProc)_glGetUniformiv)(programObj, location, params);
    }
    else
    {
	osg::notify(osg::WARN)<<"Error: glGetUniformiv not supported by OpenGL driver"<<std::endl;
    }
}

void Extensions::glGetObjectParameterfv(GLhandleARB obj, GLenum pname, GLfloat *params) const
{
    if (_glGetObjectParameterfv)
    {
	typedef void (APIENTRY * GetObjectParameterfvProc) (GLhandleARB obj, GLenum pname, GLfloat *params);
	((GetObjectParameterfvProc)_glGetObjectParameterfv)(obj, pname, params);
    }
    else
    {
	osg::notify(osg::WARN)<<"Error: glGetObjectParameterfv not supported by OpenGL driver"<<std::endl;
    }
}

void Extensions::glGetObjectParameteriv(GLhandleARB obj, GLenum pname, GLint *params) const
{
    if (_glGetObjectParameteriv)
    {
	typedef void (APIENTRY * GetObjectParameterivProc) (GLhandleARB obj, GLenum pname, GLint *params);
	((GetObjectParameterivProc)_glGetObjectParameteriv)(obj, pname, params);
    }
    else
    {
	osg::notify(osg::WARN)<<"Error: glGetObjectParameteriv not supported by OpenGL driver"<<std::endl;
    }
}

GLhandleARB Extensions::glGetHandle(GLenum pname) const
{
    if (_glGetHandle)
    {
	typedef GLhandleARB (APIENTRY * GetHandleProc) (GLenum pname);
	return ((GetHandleProc)_glGetHandle)(pname);
    }
    else
    {
	osg::notify(osg::WARN)<<"Error: glGetHandle not supported by OpenGL driver"<<std::endl;
	return -1;	//TODO
    }
}


