/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield
 *
 * This library is open source and may be redistributed and/or modified under
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * OpenSceneGraph Public License for more details.
 *
 * Some elements of GraphicsWindowWin32 have used the Producer implementation as a reference.
 * These elements are licensed under OSGPL as above, with Copyright (C) 2001-2004  Don Burns.
 */

#include <osgViewer/api/Win32/PixelBufferWin32>
#include <osgViewer/api/Win32/GraphicsWindowWin32>
#include <osg/TextureRectangle>
#include <osg/GraphicsContext>

#include <vector>
#include <map>
#include <sstream>
#include <windowsx.h>

namespace
{

static std::string sysError()
{
    DWORD stat, err = GetLastError();
    LPVOID lpMsgBuf = 0;

    stat = FormatMessage(   FORMAT_MESSAGE_ALLOCATE_BUFFER |
                     FORMAT_MESSAGE_FROM_SYSTEM |
                     FORMAT_MESSAGE_IGNORE_INSERTS,
                     NULL,
                     err,
                     MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                     (LPTSTR) &lpMsgBuf,\
                     0,NULL
                     );

    std::ostringstream msgResult;
    if ( stat > 0 && lpMsgBuf )
    {
        msgResult << (LPCTSTR)lpMsgBuf;
        LocalFree( lpMsgBuf );
    }
    else
    {
        msgResult << "Error code " << err;
    }

    return msgResult.str();
}


} // end of namespace

using namespace osgViewer;


PixelBufferWin32::PixelBufferWin32( osg::GraphicsContext::Traits* traits ):
  _initialized(false),
  _valid(false),
  _realized(false),
  _boundBuffer(0),
  _supportBindTexImage(false)
{
    _traits = traits;

    init();

    if (valid())
    {
        setState( new osg::State );
        getState()->setGraphicsContext( this );

        if (_traits.valid() && _traits->sharedContext.valid() )
        {
            getState()->setContextID( _traits->sharedContext->getState()->getContextID() );
            incrementContextIDUsageCount( getState()->getContextID() );
        }
        else
        {
            getState()->setContextID( osg::GraphicsContext::createNewContextID() );
        }
    }
}

PixelBufferWin32::~PixelBufferWin32()
{
    closeImplementation();
}

void PixelBufferWin32::init()
{
    if (_initialized) return;
    if (!_traits) return;
    if (!_traits->pbuffer) return;

    XGLIntegerAttributes attributes;
	XGLIntegerAttributes battributes;

	attributes.set(EGL_SURFACE_TYPE, EGL_PBUFFER_BIT);

	attributes.set(EGL_RED_SIZE, _traits->red);
	attributes.set(EGL_GREEN_SIZE, _traits->green);
	attributes.set(EGL_DEPTH_SIZE, _traits->depth);
	if (_traits->alpha) {
		attributes.set(EGL_ALPHA_SIZE, _traits->alpha);
	}
	if (_traits->stencil) {
		attributes.set(EGL_STENCIL_SIZE, _traits->stencil);
	}
	if (_traits->sampleBuffers) {
		attributes.set(EGL_SAMPLE_BUFFERS, _traits->sampleBuffers);
	}
	if (_traits->samples) {
		attributes.set(EGL_SAMPLES, _traits->samples);
	}

    if (_traits->target != 0 )
    {
	   battributes.set(EGL_TEXTURE_TARGET, EGL_TEXTURE_2D);
	   battributes.set(EGL_TEXTURE_FORMAT, EGL_TEXTURE_RGBA);
    }

	attributes.end();
	battributes.end();
		
	EGLConfig config;
	EGL::createDisplaySurfaceAndContextForPBuffer(_eglContextInfo, config, attributes);

	EGLint bindToTextureRGBA = 0;
	eglGetConfigAttrib(_eglContextInfo.eglDisplay, config, EGL_BIND_TO_TEXTURE_RGBA, &bindToTextureRGBA);
	_supportBindTexImage = (bindToTextureRGBA == EGL_TRUE);

    _initialized = true;
    _valid = true;

    return;
}

bool PixelBufferWin32::realizeImplementation()
{
    if (_realized)
    {
        OSG_NOTICE<<"PixelBufferWin32::realizeImplementation() Already realized"<<std::endl;
        return true;
    }

    if (!_initialized) init();

    if (!_initialized) return false;

    if ( _traits->sharedContext.valid() )
    {
		// NYI
    }

    _realized = true;
    return true;
}

void PixelBufferWin32::closeImplementation()
{
    _valid = false;
    _initialized = false;
    _hwnd = 0;
    _hdc = 0;
	_eglContextInfo.clear();
}

bool PixelBufferWin32::makeCurrentImplementation()
{
	const EGL::ContextInfo& c = getEGLContext();

	bool result = ::eglMakeCurrent(c.eglDisplay, c.eglSurface, c.eglSurface, c.eglContext);
	if (!result)
	{
		reportErrorForScreen("PixelBufferWin32::makeCurrentImplementation() - Unable to set current OpenGL rendering context", _traits->screenNum, ::GetLastError());
		return false;
	}
	OSG_NOTIFY(osg::INFO) << "eglMakeCurrent: " << c.eglDisplay << " " << c.eglSurface << " " << c.eglContext << std::endl;

    return result;
}

bool PixelBufferWin32::makeContextCurrentImplementation( GraphicsContext* readContext )
{
    GraphicsHandleWin32* graphicsHandleWin32 = dynamic_cast<GraphicsHandleWin32*>(readContext);
    if (graphicsHandleWin32)
    {
		const EGL::ContextInfo& c = graphicsHandleWin32->getEGLContext();
        return ::eglMakeCurrent(c.eglDisplay, c.eglSurface, c.eglSurface, c.eglContext);
    }
    return false;
}

bool PixelBufferWin32::releaseContextImplementation()
{
    if (!_realized)
    {
        OSG_NOTICE << "Warning: GraphicsWindow not realized, cannot do makeCurrent."<<std::endl;
        return false;
    }

	return 	::eglMakeCurrent(_eglContextInfo.eglDisplay, _eglContextInfo.eglSurface, EGL_NO_SURFACE, EGL_NO_CONTEXT);
}

void PixelBufferWin32::bindPBufferToTextureImplementation( GLenum buffer )
{
	if (!_supportBindTexImage) {
		OSG_NOTICE << "PixelBufferWin32, bind to texture image not supported" << std::endl;
		return;
	}

    int bindBuffer;

    switch (buffer)
    {
        case GL_BACK:
            bindBuffer = EGL_BACK_BUFFER;
            break;
        case GL_FRONT:
			OSG_NOTICE << "PixelBufferWin32::bindPBufferToTextureImplementation, front buffer not supported." << std::endl;
			return;
        default:
            bindBuffer = static_cast<int>(buffer);
    }

    if ( bindBuffer != _boundBuffer )
    {		
        if ( _boundBuffer != 0 && !eglReleaseTexImage(_eglContextInfo.eglDisplay, _eglContextInfo.eglSurface, _boundBuffer))
        {
            OSG_NOTICE << "PixelBufferWin32::bindPBufferToTextureImplementation, eglReleaseTexImage error: " << sysError() << std::endl;
        }

        if ( !eglBindTexImage(_eglContextInfo.eglDisplay, _eglContextInfo.eglSurface, bindBuffer))
        {
            OSG_NOTICE << "PixelBufferWin32::bindPBufferToTextureImplementation, eglBindTexImage error: " << sysError() << std::endl;
        }
        _boundBuffer = bindBuffer;
    }
}

void PixelBufferWin32::swapBuffersImplementation()
{
	eglSwapBuffers(_eglContextInfo.eglDisplay, _eglContextInfo.eglSurface);
}

