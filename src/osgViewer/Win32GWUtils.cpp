#include <osg/GLExtensions>

#include <osgViewer/api/Win32/Win32GWUtils>
#include <vector>
#include <sstream>

namespace osgViewer {

#if defined(OSG_USE_EGL)

// *** EGL Implementation

namespace EGL {

#if defined(OSG_GLES3_AVAILABLE)
	EGLint eglOpenglBit = EGL_OPENGL_ES3_BIT, eglContextClientVersion = 3;
	EGLenum eglApi = EGL_OPENGL_ES_API;
#endif
#if defined(OSG_GLES2_AVAILABLE)
	EGLint eglOpenglBit = 0, eglContextClientVersion = 2;
	EGLenum eglApi = EGL_OPENGL_ES_API;
#endif

//////////////////////////////////////////////////////////////////////////////
//              EGL::OpenGLContext implementation
//////////////////////////////////////////////////////////////////////////////

OpenGLContext::OpenGLContext() :
	_hwnd(0),
	_hdc(0),
	_eglConfig(0)
{
}

OpenGLContext::OpenGLContext(HWND hwnd, HDC hdc, EGLContext eglContext, EGLDisplay eglDisplay, EGLSurface eglSurface) :
	_hwnd(hwnd),
	_hdc(hdc),
	_eglCtx(eglContext, eglDisplay, eglSurface)
{
}

OpenGLContext::~OpenGLContext()
{
	if (_eglCtx.eglContext)
	{
		destroyContext(_eglCtx);
		OSG_NOTIFY(osg::INFO) << "eglMakeCurrent: " << _eglCtx.eglDisplay << " " << EGL_NO_SURFACE << std::endl;
	}

	if (_hdc)
	{
		::ReleaseDC(_hwnd, _hdc);
		_hdc = 0;
	}

	if (_hwnd)
	{
		::DestroyWindow(_hwnd);
		_hwnd = 0;
	}

}

void OpenGLContext::set(HWND hwnd, HDC hdc, EGLContext eglContext, EGLDisplay eglDisplay, EGLSurface eglSurface, EGLConfig eglConfig)
{
	_hwnd = hwnd;
	_hdc = hdc;
	_eglCtx.eglContext = eglContext;
	_eglCtx.eglDisplay = eglDisplay;
	_eglCtx.eglSurface = eglSurface;
	_eglConfig = eglConfig;
}

void OpenGLContext::clear() {
	_hwnd = 0;
	_hdc = 0;
	_eglCtx.clear();
}

HDC OpenGLContext::deviceContext() 
{ 
	return _hdc; 
}

EGLConfig OpenGLContext::getConfig() { 
	return _eglConfig; 
}

bool OpenGLContext::makeCurrent(HDC restoreOnHdc, bool restorePreviousOnExit)
{
	if (_hdc == 0 || _eglCtx.isEmpty()) return false;
	_previousContext = restorePreviousOnExit ? ::eglGetCurrentContext() : 0;
	_previousHdc = restoreOnHdc;

	if (_eglCtx.eglContext == _previousContext) return true;

	if (!::eglMakeCurrent(_eglCtx.eglDisplay, _eglCtx.eglSurface, _eglCtx.eglSurface, _eglCtx.eglContext))
	{
		return false;
	}
	OSG_NOTIFY(osg::DEBUG_INFO) << "eglMakeCurrent: " << _eglCtx.eglDisplay << " " << _eglCtx.eglSurface << " " << _eglCtx.eglContext << std::endl;

	return true;
}

ContextInfo& OpenGLContext::contextInfo() 
{
	return _eglCtx;
}

// ContextInfo

ContextInfo::ContextInfo() { 
	clear(); 
}

ContextInfo::ContextInfo(EGLContext _eglContext, EGLDisplay _eglDisplay, EGLSurface _eglSurface) : eglContext(_eglContext), eglDisplay(_eglDisplay), eglSurface(_eglSurface) 
{
}

ContextInfo::ContextInfo(const ContextInfo& o) : eglContext(o.eglContext), eglDisplay(o.eglDisplay), eglSurface(o.eglSurface) 
{
}

ContextInfo& ContextInfo::operator=(const ContextInfo& o) { 
	eglContext = o.eglContext; eglDisplay = o.eglDisplay; eglSurface = o.eglSurface; return *this; 
}

void ContextInfo::clear() { 
	eglContext = EGL_NO_CONTEXT; eglDisplay = EGL_NO_DISPLAY; eglSurface = EGL_NO_SURFACE; 
}

bool ContextInfo::isEmpty() { 
	return eglContext == 0 && eglDisplay == 0; 
}

EGLDisplay createAndInitializeEGLDisplay(HDC hdc = 0L)
{
	/* possible types:
	EGL_PLATFORM_ANGLE_TYPE_DEFAULT_ANGLE
	EGL_PLATFORM_ANGLE_TYPE_D3D9_ANGLE
	EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE
	EGL_PLATFORM_ANGLE_TYPE_OPENGL_ANGLE
	EGL_PLATFORM_ANGLE_TYPE_OPENGLES_ANGLE
	EGL_PLATFORM_ANGLE_TYPE_VULKAN_ANGLE
	EGL_PLATFORM_ANGLE_TYPE_NULL_ANGLE
	*/
	GLenum platformType = EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE;

	std::vector<EGLint> displayAttributes;
	displayAttributes.push_back(EGL_PLATFORM_ANGLE_TYPE_ANGLE);
	displayAttributes.push_back(platformType);
	displayAttributes.push_back(EGL_PLATFORM_ANGLE_MAX_VERSION_MAJOR_ANGLE);
	displayAttributes.push_back(EGL_DONT_CARE);
	displayAttributes.push_back(EGL_PLATFORM_ANGLE_MAX_VERSION_MINOR_ANGLE);
	displayAttributes.push_back(EGL_DONT_CARE);
	displayAttributes.push_back(EGL_PLATFORM_ANGLE_DEVICE_TYPE_ANGLE);
	displayAttributes.push_back(EGL_PLATFORM_ANGLE_DEVICE_TYPE_HARDWARE_ANGLE);
	displayAttributes.push_back(EGL_NONE);

	EGLint major, minor;
	EGLDisplay eglDisplay;
	
	if (hdc) {
		eglDisplay = eglGetPlatformDisplayEXT(EGL_PLATFORM_ANGLE_ANGLE,
			reinterpret_cast<void *>(hdc),
			displayAttributes.data());
	}
	else {
		eglDisplay = eglGetPlatformDisplayEXT(EGL_PLATFORM_ANGLE_ANGLE, reinterpret_cast<void *>(EGL_DEFAULT_DISPLAY), displayAttributes.data());
	}

	eglInitialize(eglDisplay, &major, &minor);

	OSG_NOTIFY(osg::INFO) << "EGL Version: \"" << eglQueryString(eglDisplay, EGL_VERSION) << "\"" << std::endl;
	OSG_NOTIFY(osg::INFO) << "EGL Vendor: \"" << eglQueryString(eglDisplay, EGL_VENDOR) << "\"" << std::endl;
	OSG_NOTIFY(osg::INFO) << "EGL Extensions: \"" << eglQueryString(eglDisplay, EGL_EXTENSIONS) << "\"" << std::endl;

	return eglDisplay;
}

bool createDisplaySurfaceAndContext(ContextInfo& context, EGLConfig& windowConfig, XGLAttributes<int>& configAttribs, HWND hwnd, HDC hdc)
{
	EGLDisplay eglDisplay = createAndInitializeEGLDisplay(hdc);

	configAttribs.set(EGL_RENDERABLE_TYPE, eglOpenglBit);
	configAttribs.end();

	EGLint numConfigs;
	EGLBoolean err = eglChooseConfig(eglDisplay, configAttribs.get(), &windowConfig, 1, &numConfigs);
	if (!err) {
		return false;
	}

	//EGLint surfaceAttributes[] = { EGL_RENDER_BUFFER, EGL_POST_SUB_BUFFER_SUPPORTED_NV, EGL_NONE };
	EGLint surfaceAttributes[] = { EGL_NONE, EGL_NONE };
	EGLSurface eglSurface = eglCreateWindowSurface(eglDisplay, windowConfig, hwnd, surfaceAttributes);
	eglBindAPI(eglApi);

	context.eglContext = createContext(eglDisplay, windowConfig);
	context.eglDisplay = eglDisplay;
	context.eglSurface = eglSurface;

	return true;
}

bool createDisplaySurfaceAndContextForPBuffer(ContextInfo& context, EGLConfig& config, XGLAttributes<int>& configAttribs)
{
	EGLDisplay eglDisplay = createAndInitializeEGLDisplay();
		
	configAttribs.set(EGL_RENDERABLE_TYPE, eglOpenglBit);
	configAttribs.end();

	EGLint numConfigs;
	EGLBoolean err = eglChooseConfig(eglDisplay, configAttribs.get(), &config, 1, &numConfigs);
	if (!err) {
		return false;
	}

	EGLSurface eglSurface = eglCreatePbufferSurface(eglDisplay, config, configAttribs.get());
	eglBindAPI(eglApi);

	context.eglContext = createContext(eglDisplay, config);
	context.eglDisplay = eglDisplay;
	context.eglSurface = eglSurface;

	return true;
}

EGLContext createContext(EGLDisplay eglDisplay, const EGLConfig& config)
{
	EGLint contextAttributes[] = { EGL_CONTEXT_CLIENT_VERSION, eglContextClientVersion, EGL_NONE };
	return eglCreateContext(eglDisplay, config, EGL_NO_CONTEXT, contextAttributes);
}

void destroyContext(ContextInfo& c)
{
	if (!::eglMakeCurrent(c.eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT)) {
		reportError("Win32GWUtils.destroyContext() - Unable to set current EGL rendering context", ::eglGetError());
	}
	if (!::eglDestroySurface(c.eglDisplay, c.eglSurface)) {
		reportError("Win32GWUtils.destroyContext() - Unable to destroy current EGL Surface context", ::eglGetError());
	}
	if (!::eglDestroyContext(c.eglDisplay, c.eglContext)) {
		reportError("Win32GWUtils.destroyContext() - Unable to destroy current EGL rendering context", ::eglGetError());
	}
	c.clear();
}

void preparePixelFormatSpecifications(const osg::GraphicsContext::Traits& traits,
	XGLIntegerAttributes&               attributes,
	bool                                allowSwapExchangeARB)
{
	attributes.begin();

	attributes.set(EGL_SURFACE_TYPE, EGL_WINDOW_BIT);
	attributes.set(EGL_RENDERABLE_TYPE, eglOpenglBit);

	attributes.set(EGL_RED_SIZE, traits.red);
	attributes.set(EGL_GREEN_SIZE, traits.green);
	attributes.set(EGL_BLUE_SIZE, traits.blue);
	attributes.set(EGL_DEPTH_SIZE, traits.depth);
#if 0
	if (traits.doubleBuffer)
	{
		attributes.enable(WGL_DOUBLE_BUFFER_ARB);

		switch (traits.swapMethod)
		{
		case osg::DisplaySettings::SWAP_COPY:
			attributes.set(WGL_SWAP_METHOD_ARB, WGL_SWAP_COPY_ARB);
			break;
		case osg::DisplaySettings::SWAP_EXCHANGE:
			attributes.set(WGL_SWAP_METHOD_ARB, WGL_SWAP_EXCHANGE_ARB);
			break;
		case osg::DisplaySettings::SWAP_UNDEFINED:
			attributes.set(WGL_SWAP_METHOD_ARB, WGL_SWAP_UNDEFINED_ARB);
			break;
		case osg::DisplaySettings::SWAP_DEFAULT:
			// Wojtek Lewandowski 2010-09-28:
			// Keep backward compatibility if no method is selected via traits
			// and let wglSwapExchangeARB flag select swap method.
			// However, I would rather remove this flag because its
			// now redundant to Traits::swapMethod and it looks like
			// WGL_SWAP_EXCHANGE_ARB is the GL default when no WGL_SWAP attrib is given.
			// To be precise: At least on Windows 7 and Nvidia it seems to be a default.
			if (allowSwapExchangeARB)
				attributes.set(WGL_SWAP_METHOD_ARB, WGL_SWAP_EXCHANGE_ARB);
			break;
		}
	}
#endif
	if (traits.alpha)         attributes.set(EGL_ALPHA_SIZE, traits.alpha);
	if (traits.stencil)       attributes.set(EGL_STENCIL_SIZE, traits.stencil);
	if (traits.sampleBuffers) attributes.set(EGL_SAMPLE_BUFFERS, traits.sampleBuffers);
	if (traits.samples)       attributes.set(EGL_SAMPLES, traits.samples);

	attributes.end();
}



} // end of namespace EGL

#else // OSG_USE_EGL

// *** WGL Implementation

	namespace WGL {

		//////////////////////////////////////////////////////////////////////////////
		//              WGL::OpenGLContext implementation
		//////////////////////////////////////////////////////////////////////////////

		OpenGLContext::OpenGLContext()
			: _previousHdc(0),
			_previousHglrc(0),
			_hwnd(0),
			_hdc(0),
			_hglrc(0),
			_restorePreviousOnExit(false)
		{}

		OpenGLContext::OpenGLContext(HWND hwnd, HDC hdc, HGLRC hglrc)
			: _previousHdc(0),
			_previousHglrc(0),
			_hwnd(hwnd),
			_hdc(hdc),
			_hglrc(hglrc),
			_restorePreviousOnExit(false)
		{}

		OpenGLContext::~OpenGLContext()
		{
			if(_restorePreviousOnExit && _previousHglrc != _hglrc && !::wglMakeCurrent(_previousHdc, _previousHglrc))
			{
				reportError("Win32WindowingSystem::OpenGLContext() - Unable to restore current OpenGL rendering context", ::GetLastError());
			}
			if(_hglrc)
			{
				::wglMakeCurrent(_hdc, NULL);
				::wglDeleteContext(_hglrc);
				_hglrc = 0;
			}
			_previousHdc = 0;
			_previousHglrc = 0;

			if(_hdc)
			{
				::ReleaseDC(_hwnd, _hdc);
				_hdc = 0;
			}

			if(_hwnd)
			{
				::DestroyWindow(_hwnd);
				_hwnd = 0;
			}

		}

		void OpenGLContext::set(HWND hwnd, HDC hdc, HGLRC hglrc)
		{
			_hwnd = hwnd;
			_hdc = hdc;
			_hglrc = hglrc;
		}

		void OpenGLContext::clear()
		{
			_hwnd = 0;
			_hdc = 0;
			_hglrc = 0;
		}

		HDC OpenGLContext::deviceContext()
		{
			return _hdc;
		}

		bool OpenGLContext::makeCurrent(HDC restoreOnHdc, bool restorePreviousOnExit)
		{
			if(_hdc == 0 || _hglrc == 0) return false;

			_previousHglrc = restorePreviousOnExit ? ::wglGetCurrentContext() : 0;
			_previousHdc = restoreOnHdc;

			if(_hglrc == _previousHglrc) return true;

			if(!::wglMakeCurrent(_hdc, _hglrc))
			{
				reportError("Win32WindowingSystem::OpenGLContext() - Unable to set current OpenGL rendering context", ::GetLastError());
				return false;
			}

			_restorePreviousOnExit = restorePreviousOnExit;
			return true;
		}

		void preparePixelFormatSpecifications(const osg::GraphicsContext::Traits& traits,
			XGLIntegerAttributes&               attributes,
			bool                                allowSwapExchangeARB)
		{
			attributes.begin();

			attributes.enable(WGL_DRAW_TO_WINDOW_ARB);
			attributes.enable(WGL_SUPPORT_OPENGL_ARB);

			attributes.set(WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB);
			attributes.set(WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB);

			attributes.set(WGL_COLOR_BITS_ARB, traits.red + traits.green + traits.blue);
			attributes.set(WGL_RED_BITS_ARB, traits.red);
			attributes.set(WGL_GREEN_BITS_ARB, traits.green);
			attributes.set(WGL_BLUE_BITS_ARB, traits.blue);
			attributes.set(WGL_DEPTH_BITS_ARB, traits.depth);

			if(traits.doubleBuffer)
			{
				attributes.enable(WGL_DOUBLE_BUFFER_ARB);

				switch(traits.swapMethod)
				{
				case osg::DisplaySettings::SWAP_COPY:
					attributes.set(WGL_SWAP_METHOD_ARB, WGL_SWAP_COPY_ARB);
					break;
				case osg::DisplaySettings::SWAP_EXCHANGE:
					attributes.set(WGL_SWAP_METHOD_ARB, WGL_SWAP_EXCHANGE_ARB);
					break;
				case osg::DisplaySettings::SWAP_UNDEFINED:
					attributes.set(WGL_SWAP_METHOD_ARB, WGL_SWAP_UNDEFINED_ARB);
					break;
				case osg::DisplaySettings::SWAP_DEFAULT:
					// Wojtek Lewandowski 2010-09-28:
					// Keep backward compatibility if no method is selected via traits
					// and let wglSwapExchangeARB flag select swap method.
					// However, I would rather remove this flag because its
					// now redundant to Traits::swapMethod and it looks like
					// WGL_SWAP_EXCHANGE_ARB is the GL default when no WGL_SWAP attrib is given.
					// To be precise: At least on Windows 7 and Nvidia it seems to be a default.
					if(allowSwapExchangeARB)
						attributes.set(WGL_SWAP_METHOD_ARB, WGL_SWAP_EXCHANGE_ARB);
					break;
				}
			}

			if(traits.alpha)         attributes.set(WGL_ALPHA_BITS_ARB, traits.alpha);
			if(traits.stencil)       attributes.set(WGL_STENCIL_BITS_ARB, traits.stencil);
			if(traits.sampleBuffers) attributes.set(WGL_SAMPLE_BUFFERS_ARB, traits.sampleBuffers);
			if(traits.samples)       attributes.set(WGL_SAMPLES_ARB, traits.samples);

			if(traits.quadBufferStereo) attributes.enable(WGL_STEREO_ARB);

			attributes.end();
		}
	} // end of namespace WGL
#endif // OSG_USE_EGL


///////////////////////////////////////////////////////////////////////////////
//                             Error reporting
//////////////////////////////////////////////////////////////////////////////

void reportError(const std::string& msg)
{
	OSG_WARN << "Error: " << msg.c_str() << std::endl;
}

void reportError(const std::string& msg, unsigned int errorCode)
{
	//
	// Some APIs are documented as returning the error in ::GetLastError but apparently do not
	// Skip "Reason" field if the errorCode is still success
	//

	if (errorCode == 0)
	{
		reportError(msg);
		return;
	}

	OSG_WARN << "Windows Error #" << errorCode << ": " << msg.c_str();

	LPVOID lpMsgBuf;

	if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		errorCode,
		0, // Default language
		(LPTSTR)&lpMsgBuf,
		0,
		NULL) != 0)
	{
		OSG_WARN << ". Reason: " << LPTSTR(lpMsgBuf) << std::endl;
		::LocalFree(lpMsgBuf);
	}
	else
	{
		OSG_WARN << std::endl;
	}
}

void reportErrorForScreen(const std::string& msg, const osg::GraphicsContext::ScreenIdentifier& si, unsigned int errorCode)
{
	std::ostringstream str;

	str << "[Screen #" << si.screenNum << "] " << msg;
	reportError(str.str(), errorCode);
}

} // end of namespace osgViewer