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

#if !defined(OSG_USE_EGL)
#include <osgViewer/api/Win32/Win32GWUtils>
#include <osgViewer/api/Win32/PixelBufferWin32>
#include <osgViewer/api/Win32/GraphicsWindowWin32>
#include <osg/TextureRectangle>

#include <vector>
#include <map>
#include <sstream>

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


    static int __tempwnd_id = 0;
class TemporaryWindow: public osg::Referenced
{
public:
    TemporaryWindow():
        _handle(0),
        _dc(0),
        _context(0),
        _instance(0)
    {
        create();
    }

    HWND  getHandle() const    { return _handle; }
    HDC   getDC() const        { return _dc; }
    HGLRC getContext() const   { return _context; }

    bool makeCurrent();

protected:
    ~TemporaryWindow();

    TemporaryWindow(const TemporaryWindow &):
        _handle(0),
        _dc(0),
        _context(0),
        _instance(0) {}

    TemporaryWindow &operator=(const TemporaryWindow &) { return *this; }

    void create();
    void kill();

private:
    HWND _handle;
    HDC _dc;
    HGLRC _context;
    HINSTANCE _instance;
    std::string _classname;
};

void TemporaryWindow::create()
{
    std::ostringstream oss;
    oss << "tempwnd" << (++__tempwnd_id);
    _classname = oss.str();

    _instance = GetModuleHandle(0);

    WNDCLASS wndclass;

    wndclass.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wndclass.lpfnWndProc   = DefWindowProc;
    wndclass.cbClsExtra    = 0;
    wndclass.cbWndExtra    = 0;
    wndclass.hInstance     = _instance;
    wndclass.hCursor       = 0;
    wndclass.hIcon         = 0;
    wndclass.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wndclass.lpszMenuName  = 0;
    wndclass.lpszClassName = _classname.c_str();

    if (!RegisterClass(&wndclass))
        return;

    if (!(_handle = CreateWindowEx( 0,
                                    _classname.c_str(),
                                    TEXT(_classname.c_str()),
                                    WS_POPUP,
                                    0,
                                    0,
                                    100,
                                    100,
                                    0,
                                    0,
                                    _instance,
                                    0)))
    {
        OSG_WARN << "PixelBufferWin32, could not create temporary window: " << sysError() << std::endl;
        kill();
        return;
    }

    if (!(_dc = GetDC(_handle)))
    {
        OSG_WARN << "PixelBufferWin32, could not get device context for temporary window: " << sysError() << std::endl;
        kill();
        return;
    }

    PIXELFORMATDESCRIPTOR pfd = {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW |
        PFD_SUPPORT_OPENGL,
        PFD_TYPE_RGBA,
        24,
        0, 0, 0, 0, 0, 0,
        0,
        0,
        0,
        0, 0, 0, 0,
        16,
        0,
        0,
        PFD_MAIN_PLANE,
        0,
        0, 0, 0
    };

    int visual_id = ChoosePixelFormat(_dc, &pfd);

    if (!SetPixelFormat(_dc, visual_id, &pfd))
    {
        OSG_WARN << "PixelBufferWin32, could not set pixel format for temporary window: " << sysError() << std::endl;
        kill();
        return;
    }

    if (!(_context = wglCreateContext(_dc)))
    {
        OSG_WARN << "PixelBufferWin32, could not get graphics context for temporary window: " << sysError() << std::endl;
        kill();
        return;
    }
}

TemporaryWindow::~TemporaryWindow()
{
    kill();
}

void TemporaryWindow::kill()
{
    if (_context)
    {
        // mew 2005-05-09 commented out due to crashes.
        // possible causes are unsafe destructor ordering, or context already
        // deleted by window deletion; see:
        // http://openscenegraph.org/pipermail/osg-users/2005-May/052753.html
        //wglDeleteContext(_context);
        _context = 0;
    }

    if (_dc)
    {
        ReleaseDC(_handle, _dc);
        _dc = 0;
    }

    if (_handle)
    {
        DestroyWindow(_handle);
        _handle = 0;
    }

    UnregisterClass(_classname.c_str(), _instance);
    _instance = 0;
}

bool TemporaryWindow::makeCurrent()
{
    bool result = wglMakeCurrent(_dc, _context) == TRUE ? true : false;
    if (!result)
    {
        OSG_NOTICE << "PixelBufferWin32, could not make the temporary window's context active: " << sysError() << std::endl;
    }
    return result;
}

class WGLExtensions : public osg::Referenced
{
public:
    typedef HPBUFFERARB (WINAPI * WGLCreatePBufferProc)    (HDC hDC, int iPixelFormat, int iWidth, int iHeight, const int *piAttribList);
    typedef HDC         (WINAPI * WGLGetPBufferDCProc)     (HPBUFFERARB hPbuffer);
    typedef int         (WINAPI * WGLReleasePBufferDCProc) (HPBUFFERARB hPbuffer, HDC hDC);
    typedef bool        (WINAPI * WGLDestroyPBufferProc)   (HPBUFFERARB hPbuffer);
    typedef bool        (WINAPI * WGLQueryPBufferProc)     (HPBUFFERARB hPbuffer, int iAttribute, int *piValue);
    typedef bool        (WINAPI * WGLBindTexImageProc)     (HPBUFFERARB hPbuffer, int iBuffer);
    typedef bool        (WINAPI * WGLReleaseTexImageProc)  (HPBUFFERARB hPbuffer, int iBuffer);
    typedef bool        (WINAPI * WGLSetPbufferAttribProc) (HPBUFFERARB hPbuffer, const int * piAttribList);
    typedef bool        (WINAPI * WGLChoosePixelFormatProc) (HDC, const int *, const float *, unsigned int, int *, unsigned int *);
    typedef bool        (WINAPI * WGLMakeContextCurrentProc) (HDC, HDC, HGLRC);

    WGLCreatePBufferProc        wglCreatePbufferARB;
    WGLGetPBufferDCProc            wglGetPbufferDCARB;
    WGLReleasePBufferDCProc        wglReleasePbufferDCARB;
    WGLDestroyPBufferProc        wglDestroyPbufferARB;
    WGLQueryPBufferProc            wglQueryPbufferARB;
    WGLBindTexImageProc            wglBindTexImageARB;
    WGLReleaseTexImageProc        wglReleaseTexImageARB;
    WGLChoosePixelFormatProc    wglChoosePixelFormatARB;
    WGLMakeContextCurrentProc    wglMakeContextCurrentARB;

    static WGLExtensions *instance();

    bool isValid();

protected:
    WGLExtensions();
    ~WGLExtensions();

private:
    static std::map<HGLRC, osg::ref_ptr<WGLExtensions> > _instances;
};

std::map<HGLRC, osg::ref_ptr<WGLExtensions> > WGLExtensions::_instances;
WGLExtensions::WGLExtensions()
{
    wglCreatePbufferARB     = (WGLCreatePBufferProc)wglGetProcAddress("wglCreatePbufferARB");
    wglGetPbufferDCARB      = (WGLGetPBufferDCProc)wglGetProcAddress("wglGetPbufferDCARB");
    wglReleasePbufferDCARB  = (WGLReleasePBufferDCProc)wglGetProcAddress("wglReleasePbufferDCARB");
    wglDestroyPbufferARB    = (WGLDestroyPBufferProc)wglGetProcAddress("wglDestroyPbufferARB");
    wglQueryPbufferARB      = (WGLQueryPBufferProc)wglGetProcAddress("wglQueryPbufferARB");
    wglBindTexImageARB      = (WGLBindTexImageProc)wglGetProcAddress("wglBindTexImageARB");
    wglReleaseTexImageARB   = (WGLReleaseTexImageProc)wglGetProcAddress("wglReleaseTexImageARB");
    wglChoosePixelFormatARB = (WGLChoosePixelFormatProc)wglGetProcAddress("wglChoosePixelFormatARB");
    wglMakeContextCurrentARB = (WGLMakeContextCurrentProc)wglGetProcAddress("wglMakeContextCurrentARB");
    if (!wglMakeContextCurrentARB)
    {
        wglMakeContextCurrentARB = (WGLMakeContextCurrentProc)wglGetProcAddress("wglMakeContextCurrentEXT");
    }
}

WGLExtensions::~WGLExtensions()
{
}

bool WGLExtensions::isValid()
{
    return (wglCreatePbufferARB && wglGetPbufferDCARB && wglReleasePbufferDCARB && wglDestroyPbufferARB &&
        wglQueryPbufferARB && wglChoosePixelFormatARB );
}

WGLExtensions *WGLExtensions::instance()
{
    HGLRC context = wglGetCurrentContext();

    // Get wgl function pointers for the current graphics context, or if there is no
    // current context then use a temporary window.

    if (!_instances[context])
    {
        if ( context == 0 )
        {
            osg::ref_ptr<TemporaryWindow> tempWin= new TemporaryWindow;
            tempWin->makeCurrent();
            _instances[HGLRC(0)] = new WGLExtensions;
        }
        else
        {
            _instances[context] = new WGLExtensions;
        }
    }

    return _instances[context].get();
}


}

using namespace osgViewer;


PixelBufferWin32::PixelBufferWin32( osg::GraphicsContext::Traits* traits ):
  _initialized(false),
  _valid(false),
  _realized(false),
  _boundBuffer(0)
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

    WGLExtensions* wgle = WGLExtensions::instance();

    if (!wgle || !wgle->isValid())
    {
        OSG_NOTICE << "PixelBufferWin32::init(), Error: some wgl extensions not supported" << std::endl;
        return;
    }

    std::vector<int> fAttribList;
    std::vector<int> bAttribList;

    fAttribList.push_back(WGL_DRAW_TO_PBUFFER_ARB);
    fAttribList.push_back(true);
    fAttribList.push_back(WGL_SUPPORT_OPENGL_ARB);
    fAttribList.push_back(true);
    fAttribList.push_back(WGL_PIXEL_TYPE_ARB);
    fAttribList.push_back(WGL_TYPE_RGBA_ARB);

    bAttribList.push_back(WGL_PBUFFER_LARGEST_ARB);
    bAttribList.push_back(true);

    fAttribList.push_back(WGL_RED_BITS_ARB);
    fAttribList.push_back(_traits->red);
    fAttribList.push_back(WGL_GREEN_BITS_ARB);
    fAttribList.push_back(_traits->green);
    fAttribList.push_back(WGL_BLUE_BITS_ARB);
    fAttribList.push_back(_traits->blue);
    if (_traits->alpha)
    {
        fAttribList.push_back(WGL_ALPHA_BITS_ARB);
        fAttribList.push_back(_traits->alpha);
    }

    fAttribList.push_back(WGL_DEPTH_BITS_ARB);
    fAttribList.push_back(_traits->depth);

    if (_traits->stencil)
    {
        fAttribList.push_back(WGL_STENCIL_BITS_ARB);
        fAttribList.push_back(_traits->stencil);
    }

    if (_traits->sampleBuffers)
    {
        fAttribList.push_back(WGL_SAMPLE_BUFFERS_ARB);
        fAttribList.push_back(_traits->sampleBuffers);

        fAttribList.push_back(WGL_SAMPLES_ARB);
        fAttribList.push_back(_traits->samples);
    }

    if (_traits->doubleBuffer)
    {
        fAttribList.push_back(WGL_DOUBLE_BUFFER_ARB);
        fAttribList.push_back(true);
    }

    if (_traits->target != 0 && wgle->wglBindTexImageARB )
    {
        // TODO: Cube Maps
       if (_traits->target == GL_TEXTURE_RECTANGLE)
       {
            bAttribList.push_back(WGL_TEXTURE_TARGET_ARB);
            bAttribList.push_back(WGL_TEXTURE_RECTANGLE_NV);

            if (_traits->alpha)
                fAttribList.push_back(WGL_BIND_TO_TEXTURE_RECTANGLE_RGBA_NV);
            else
                fAttribList.push_back(WGL_BIND_TO_TEXTURE_RECTANGLE_RGB_NV);
            fAttribList.push_back(true);

       }
       else
       {
            bAttribList.push_back(WGL_TEXTURE_TARGET_ARB);
            bAttribList.push_back(WGL_TEXTURE_2D_ARB);

            if (_traits->alpha)
                fAttribList.push_back(WGL_BIND_TO_TEXTURE_RGBA_ARB);
            else
                fAttribList.push_back(WGL_BIND_TO_TEXTURE_RGB_ARB);
            fAttribList.push_back(true);

       }

        bAttribList.push_back(WGL_TEXTURE_FORMAT_ARB);
        if (_traits->alpha)
            bAttribList.push_back(WGL_TEXTURE_RGBA_ARB);
        else
            bAttribList.push_back(WGL_TEXTURE_RGB_ARB);

        if (_traits->mipMapGeneration)
        {
            fAttribList.push_back(WGL_MIPMAP_TEXTURE_ARB);
            fAttribList.push_back(true);
        }
    }

    fAttribList.push_back(0);
    bAttribList.push_back(0);

    HDC hdc = 0;
    int format;
    osg::ref_ptr<TemporaryWindow> tempWin;

    tempWin = new TemporaryWindow;
    hdc = tempWin->getDC();
    tempWin->makeCurrent();

    wgle = WGLExtensions::instance();

    unsigned int nformats = 0;
    wgle->wglChoosePixelFormatARB(hdc, &fAttribList[0], NULL, 1, &format, &nformats);
    if (nformats == 0)
    {
        OSG_NOTICE << "PixelBufferWin32::init(), Error: Couldn't find a suitable pixel format" << std::endl;
        return;
    }

    _hwnd = reinterpret_cast<HWND>(wgle->wglCreatePbufferARB(hdc, format, _traits->width, _traits->height, &bAttribList[0]));
    if (!_hwnd)
    {
        OSG_NOTICE << "PixelBufferWin32::init, wglCreatePbufferARB error: " << sysError() << std::endl;
        return ;
    }

    _hdc = wgle->wglGetPbufferDCARB(reinterpret_cast<HPBUFFERARB>(_hwnd));
    if (!_hdc)
    {
        OSG_NOTICE << "PixelBufferWin32::init, wglGetPbufferDCARB error: " << sysError() << std::endl;
        return;
    }

    _hglrc = wglCreateContext(_hdc);
    if (!_hglrc)
    {
        OSG_NOTICE << "PixelBufferWin32::init, wglCreateContext error: " << sysError() << std::endl;
        return;
    }

    int iWidth = 0;
    int iHeight = 0;
    wgle->wglQueryPbufferARB(reinterpret_cast<HPBUFFERARB>(_hwnd), WGL_PBUFFER_WIDTH_ARB, &iWidth);
    wgle->wglQueryPbufferARB(reinterpret_cast<HPBUFFERARB>(_hwnd), WGL_PBUFFER_HEIGHT_ARB, &iHeight);

    if (_traits->width != iWidth || _traits->height != iHeight)
    {
        OSG_NOTICE << "PixelBufferWin32::init(), pbuffer created with different size then requested" << std::endl;
        OSG_NOTICE << "\tRequested size (" << _traits->width << "," << _traits->height << ")" << std::endl;
        OSG_NOTICE << "\tPbuffer size (" << iWidth << "," << iHeight << ")" << std::endl;
        _traits->width  = iWidth;
        _traits->height = iHeight;
    }

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
        GraphicsHandleWin32* graphicsHandleWin32 = dynamic_cast<GraphicsHandleWin32*>(_traits->sharedContext.get());
        if (graphicsHandleWin32)
        {
            if ( !wglShareLists(graphicsHandleWin32->getWGLContext(), _hglrc) )
            {
                OSG_NOTICE << "PixelBufferWin32::realizeImplementation, wglShareLists error: " << sysError() << std::endl;
            }
        }
    }

    _realized = true;
    return true;
}

void PixelBufferWin32::closeImplementation()
{
    if (_hwnd)
    {
        WGLExtensions* wgle = WGLExtensions::instance();

        wglMakeCurrent(NULL,NULL);

        if ( !wglDeleteContext(_hglrc) )
        {
            OSG_NOTICE << "PixelBufferWin32::closeImplementation, wglDeleteContext error: " << sysError() << std::endl;
        }

        if (wgle && wgle->isValid())
        {
            // Note that closeImplementation() should only be called from the same thread as created the pbuffer,
            // otherwise these routines will return an error.

            if ( !wgle->wglReleasePbufferDCARB(reinterpret_cast<HPBUFFERARB>(_hwnd), _hdc) )
            {
                OSG_NOTICE << "PixelBufferWin32::closeImplementation, wglReleasePbufferDCARB error: " << sysError() << std::endl;
            }
            if ( !wgle->wglDestroyPbufferARB(reinterpret_cast<HPBUFFERARB>(_hwnd)) )
            {
                OSG_NOTICE << "PixelBufferWin32::closeImplementation, wglDestroyPbufferARB error: " << sysError() << std::endl;
            }
        }
    }
    _valid = false;
    _initialized = false;
    _hwnd = 0;
    _hdc = 0;
    _hglrc = 0;
}

bool PixelBufferWin32::makeCurrentImplementation()
{
    bool result = wglMakeCurrent(_hdc, _hglrc)==TRUE?true:false;
    if (!result)
    {
        OSG_NOTICE << "PixelBufferWin32::makeCurrentImplementation, wglMakeCurrent error: " << sysError() << std::endl;
    }

    // If the pbuffer is bound to a texture then release it.  This operation requires a current context, so
    // do it after the MakeCurrent.

    if ( _boundBuffer!=0 )
    {
        WGLExtensions* wgle = WGLExtensions::instance();
        if ( wgle && wgle->wglReleaseTexImageARB )
        {
            if ( !wgle->wglReleaseTexImageARB(reinterpret_cast<HPBUFFERARB>(_hwnd), _boundBuffer) )
            {
                OSG_NOTICE << "PixelBufferWin32::makeCurrentImplementation, wglReleaseTexImageARB error: " << sysError() << std::endl;
            }
            _boundBuffer=0;
        }
    }

    return result;
}

bool PixelBufferWin32::makeContextCurrentImplementation( GraphicsContext* readContext )
{
    WGLExtensions* wgle = WGLExtensions::instance();

    if ( !wgle || !wgle->wglMakeContextCurrentARB )
    {
        OSG_NOTICE << "PixelBufferWin32, wglMakeContextCurrentARB not available" << std::endl;
        return false;
    }

    GraphicsHandleWin32* graphicsHandleWin32 = dynamic_cast<GraphicsHandleWin32*>(readContext);
    if (graphicsHandleWin32)
    {
        return wgle->wglMakeContextCurrentARB(_hdc, graphicsHandleWin32->getHDC(), _hglrc);
    }
    return false;
}

bool PixelBufferWin32::releaseContextImplementation()
{
    if (!_realized)
    {
        OSG_NOTICE<<"Warning: GraphicsWindow not realized, cannot do makeCurrent."<<std::endl;
        return false;
    }

    return wglMakeCurrent( _hdc, 0 ) == TRUE?true:false;
}

void PixelBufferWin32::bindPBufferToTextureImplementation( GLenum buffer )
{
    WGLExtensions* wgle = WGLExtensions::instance();

    if ( !wgle || !wgle->wglBindTexImageARB )
    {
        OSG_NOTICE << "PixelBufferWin32, wglBindTexImageARB not available" << std::endl;
        return;
    }

    int bindBuffer;

    switch (buffer)
    {
        case GL_BACK:
            bindBuffer = WGL_BACK_LEFT_ARB;
            break;
        case GL_FRONT:
            bindBuffer = WGL_FRONT_LEFT_ARB;
            break;
        default:
            bindBuffer = static_cast<int>(buffer);
    }

    if ( bindBuffer != _boundBuffer )
    {
        if ( _boundBuffer != 0 && !wgle->wglReleaseTexImageARB(reinterpret_cast<HPBUFFERARB>(_hwnd), _boundBuffer) )
        {
            OSG_NOTICE << "PixelBufferWin32::bindPBufferToTextureImplementation, wglReleaseTexImageARB error: " << sysError() << std::endl;
        }

        if ( !wgle->wglBindTexImageARB(reinterpret_cast<HPBUFFERARB>(_hwnd), bindBuffer) )
        {
            OSG_NOTICE << "PixelBufferWin32::bindPBufferToTextureImplementation, wglBindTexImageARB error: " << sysError() << std::endl;
        }
        _boundBuffer = bindBuffer;
    }
}

void PixelBufferWin32::swapBuffersImplementation()
{
    SwapBuffers( _hdc );
}

#endif // OSG_USE_EGL
