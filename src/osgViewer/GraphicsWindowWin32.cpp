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
 * This file is Copyright (C) 2007 - André Garneau (andre@pixdev.com) and licensed under OSGPL.
 *
 * Some elements of GraphicsWindowWin32 have used the Producer implementation as a reference.
 * These elements are licensed under OSGPL as above, with Copyright (C) 2001-2004  Don Burns.
 */

#include <osgViewer/api/Win32/GraphicsWindowWin32>
#include <osgViewer/api/Win32/PixelBufferWin32>
#include <osgViewer/View>

#include <osg/GL>
#include <osg/DeleteHandler>
#include <osg/ApplicationUsage>

#include <vector>
#include <map>
#include <sstream>
#include <windowsx.h>

#if(WINVER < 0x0601)
// Provide Declarations for Multitouch

#define WM_TOUCH                        0x0240

/*
 * Touch Input defines and functions
 */

/*
 * Touch input handle
 */
DECLARE_HANDLE(HTOUCHINPUT);

typedef struct tagTOUCHINPUT {
    LONG x;
    LONG y;
    HANDLE hSource;
    DWORD dwID;
    DWORD dwFlags;
    DWORD dwMask;
    DWORD dwTime;
    ULONG_PTR dwExtraInfo;
    DWORD cxContact;
    DWORD cyContact;
} TOUCHINPUT, *PTOUCHINPUT;
typedef TOUCHINPUT const * PCTOUCHINPUT;


/*
 * Conversion of touch input coordinates to pixels
 */
#define TOUCH_COORD_TO_PIXEL(l)         ((l) / 100)

/*
 * Touch input flag values (TOUCHINPUT.dwFlags)
 */
#define TOUCHEVENTF_MOVE            0x0001
#define TOUCHEVENTF_DOWN            0x0002
#define TOUCHEVENTF_UP              0x0004
#define TOUCHEVENTF_INRANGE         0x0008
#define TOUCHEVENTF_PRIMARY         0x0010
#define TOUCHEVENTF_NOCOALESCE      0x0020
#define TOUCHEVENTF_PEN             0x0040
#define TOUCHEVENTF_PALM            0x0080

#endif

typedef
BOOL
(WINAPI GetTouchInputInfoFunc)(
    HTOUCHINPUT hTouchInput,               // input event handle; from touch message lParam
    UINT cInputs,                          // number of elements in the array
    PTOUCHINPUT pInputs,  // array of touch inputs
    int cbSize);                           // sizeof(TOUCHINPUT)

typedef
BOOL
(WINAPI CloseTouchInputHandleFunc(
    HTOUCHINPUT hTouchInput));                   // input event handle; from touch message lParam

typedef
BOOL
(WINAPI RegisterTouchWindowFunc(
    HWND hwnd,
    ULONG ulFlags));

// Declared static in order to get Header File clean
static RegisterTouchWindowFunc *registerTouchWindowFunc = NULL;
static CloseTouchInputHandleFunc *closeTouchInputHandleFunc = NULL;
static GetTouchInputInfoFunc *getTouchInputInfoFunc = NULL;

using namespace osgViewer;

namespace osgViewer
{

static osg::ApplicationUsageProxy GraphicsWindowWin32_e0(osg::ApplicationUsage::ENVIRONMENTAL_VARIABLE,"OSG_WIN32_NV_MULTIMON_MULTITHREAD_WORKAROUND on/off","Enable/disable duplicate makeCurrentContext call used as workaround for WinXP/NVidia/MultiView/MulitThread isues (pre 178.13 drivers).");

//
// Defines from the WGL_ARB_pixel_format specification document
// See http://www.opengl.org/registry/specs/ARB/wgl_pixel_format.txt
//

#define WGL_NUMBER_PIXEL_FORMATS_ARB            0x2000
#define WGL_DRAW_TO_WINDOW_ARB                  0x2001
#define WGL_DRAW_TO_BITMAP_ARB                  0x2002
#define WGL_ACCELERATION_ARB                    0x2003
#define WGL_NEED_PALETTE_ARB                    0x2004
#define WGL_NEED_SYSTEM_PALETTE_ARB             0x2005
#define WGL_SWAP_LAYER_BUFFERS_ARB              0x2006
#define WGL_SWAP_METHOD_ARB                     0x2007
#define WGL_NUMBER_OVERLAYS_ARB                 0x2008
#define WGL_NUMBER_UNDERLAYS_ARB                0x2009
#define WGL_TRANSPARENT_ARB                     0x200A
#define WGL_TRANSPARENT_RED_VALUE_ARB           0x2037
#define WGL_TRANSPARENT_GREEN_VALUE_ARB         0x2038
#define WGL_TRANSPARENT_BLUE_VALUE_ARB          0x2039
#define WGL_TRANSPARENT_ALPHA_VALUE_ARB         0x203A
#define WGL_TRANSPARENT_INDEX_VALUE_ARB         0x203B
#define WGL_SHARE_DEPTH_ARB                     0x200C
#define WGL_SHARE_STENCIL_ARB                   0x200D
#define WGL_SHARE_ACCUM_ARB                     0x200E
#define WGL_SUPPORT_GDI_ARB                     0x200F
#define WGL_SUPPORT_OPENGL_ARB                  0x2010
#define WGL_DOUBLE_BUFFER_ARB                   0x2011
#define WGL_STEREO_ARB                          0x2012
#define WGL_PIXEL_TYPE_ARB                      0x2013
#define WGL_COLOR_BITS_ARB                      0x2014
#define WGL_RED_BITS_ARB                        0x2015
#define WGL_RED_SHIFT_ARB                       0x2016
#define WGL_GREEN_BITS_ARB                      0x2017
#define WGL_GREEN_SHIFT_ARB                     0x2018
#define WGL_BLUE_BITS_ARB                       0x2019
#define WGL_BLUE_SHIFT_ARB                      0x201A
#define WGL_ALPHA_BITS_ARB                      0x201B
#define WGL_ALPHA_SHIFT_ARB                     0x201C
#define WGL_ACCUM_BITS_ARB                      0x201D
#define WGL_ACCUM_RED_BITS_ARB                  0x201E
#define WGL_ACCUM_GREEN_BITS_ARB                0x201F
#define WGL_ACCUM_BLUE_BITS_ARB                 0x2020
#define WGL_ACCUM_ALPHA_BITS_ARB                0x2021
#define WGL_DEPTH_BITS_ARB                      0x2022
#define WGL_STENCIL_BITS_ARB                    0x2023
#define WGL_AUX_BUFFERS_ARB                     0x2024
#define WGL_NO_ACCELERATION_ARB                 0x2025
#define WGL_GENERIC_ACCELERATION_ARB            0x2026
#define WGL_FULL_ACCELERATION_ARB               0x2027
#define WGL_SWAP_EXCHANGE_ARB                   0x2028
#define WGL_SWAP_COPY_ARB                       0x2029
#define WGL_SWAP_UNDEFINED_ARB                  0x202A
#define WGL_TYPE_RGBA_ARB                       0x202B
#define WGL_TYPE_COLORINDEX_ARB                 0x202C
#define WGL_SAMPLE_BUFFERS_ARB                  0x2041
#define WGL_SAMPLES_ARB                         0x2042

#ifndef WGL_ARB_create_context
#define WGL_CONTEXT_DEBUG_BIT_ARB      0x00000001
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB 0x00000002
#define WGL_CONTEXT_MAJOR_VERSION_ARB  0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB  0x2092
#define WGL_CONTEXT_LAYER_PLANE_ARB    0x2093
#define WGL_CONTEXT_FLAGS_ARB          0x2094
#define WGL_CONTEXT_PROFILE_MASK_ARB   0x9126
#define ERROR_INVALID_VERSION_ARB      0x2095
#endif

#ifndef WGL_ARB_create_context
#define WGL_ARB_create_context 1
#ifdef WGL_WGLEXT_PROTOTYPES
extern HGLRC WINAPI wglCreateContextAttribsARB (HDC, HGLRC, const int *);
#endif /* WGL_WGLEXT_PROTOTYPES */
typedef HGLRC (WINAPI * PFNWGLCREATECONTEXTATTRIBSARBPROC) (HDC hDC, HGLRC hShareContext, const int *attribList);
#endif

//
// Entry points used from the WGL extensions
//
//    BOOL wglChoosePixelFormatARB(HDC hdc,
//                                 const int *piAttribIList,
//                                 const FLOAT *pfAttribFList,
//                                 UINT nMaxFormats,
//                                 int *piFormats,
//                                 UINT *nNumFormats);
//

typedef bool (WINAPI * WGLChoosePixelFormatARB) ( HDC, const int *, const float *, unsigned int, int *, unsigned int * );

//
// Utility class to specify the visual attributes for wglChoosePixelFormatARB() function
//

template <typename T> class WGLAttributes
{
  public:

      WGLAttributes()  {}
      ~WGLAttributes() {}

      void begin()                              { m_parameters.clear(); }
      void set( const T& id, const T& value )   { add(id); add(value); }
      void enable( const T& id )                { add(id); add(true); }
      void disable( const T& id )               { add(id); add(false); }
      void end()                                { add(0); }

      const T* get() const                      { return &m_parameters.front(); }

  protected:

      void add( const T& t )                    { m_parameters.push_back(t); }

      std::vector<T>    m_parameters;        // parameters added

  private:

      // No implementation for these
      WGLAttributes( const WGLAttributes& );
      WGLAttributes& operator=( const WGLAttributes& );
};

typedef WGLAttributes<int>     WGLIntegerAttributes;
typedef WGLAttributes<float> WGLFloatAttributes;

//
// Class responsible for interfacing with the Win32 Window Manager
// The behavior of this class is specific to OSG needs and is not a
// generic Windowing interface.
//
// NOTE: This class is intended to be used by a single-thread.
//         Multi-threading is not enabled for performance reasons.
//         The creation/deletion of graphics windows should be done
//         by a single controller thread. That thread should then
//         call the checkEvents() method of all created windows periodically.
//         This is the case with OSG as a "main" thread does all
//         setup, update & event processing. Rendering is done (optionally) by other threads.
//
// !@todo Have a dedicated thread managed by the Win32WindowingSystem class handle the
//        creation and event message processing for all windows it manages. This
//        is to relieve the "main" thread from having to do this synchronously
//        during frame generation. The "main" thread would only have to process
//          each osgGA-type window event queue.
//

class Win32WindowingSystem : public osg::GraphicsContext::WindowingSystemInterface
{
  public:

    // A class representing an OpenGL rendering context
    class OpenGLContext
    {
      public:

        OpenGLContext()
        : _previousHdc(0),
          _previousHglrc(0),
          _hwnd(0),
          _hdc(0),
          _hglrc(0),
          _restorePreviousOnExit(false)
        {}

        OpenGLContext( HWND hwnd, HDC hdc, HGLRC hglrc )
        : _previousHdc(0),
          _previousHglrc(0),
          _hwnd(hwnd),
          _hdc(hdc),
          _hglrc(hglrc),
          _restorePreviousOnExit(false)
        {}

        ~OpenGLContext();

        void set( HWND hwnd, HDC hdc, HGLRC hglrc )
        {
            _hwnd  = hwnd;
            _hdc   = hdc;
            _hglrc = hglrc;
        }

        HDC deviceContext() { return _hdc; }

        bool makeCurrent( HDC restoreOnHdc, bool restorePreviousOnExit );

      protected:

        //
        // Data members
        //

        HDC   _previousHdc;                // previously HDC to restore rendering context on
        HGLRC _previousHglrc;           // previously current rendering context
        HWND  _hwnd;                    // handle to OpenGL window
        HDC   _hdc;                     // handle to device context
        HGLRC _hglrc;                   // handle to OpenGL rendering context
        bool  _restorePreviousOnExit;   // restore original context on exit

        private:

        // no implementation for these
        OpenGLContext( const OpenGLContext& );
        OpenGLContext& operator=( const OpenGLContext& );
    };

    static std::string osgGraphicsWindowWithCursorClass;    //!< Name of Win32 window class (with cursor) used by OSG graphics window instances
    static std::string osgGraphicsWindowWithoutCursorClass; //!< Name of Win32 window class (without cursor) used by OSG graphics window instances

    Win32WindowingSystem();
    ~Win32WindowingSystem();

    // Access the Win32 windowing system through this singleton class.
    static Win32WindowingSystem* getInterface()
    {
        static Win32WindowingSystem* win32Interface = new Win32WindowingSystem;
        return win32Interface;
    }

    // Return the number of screens present in the system
    virtual unsigned int getNumScreens( const osg::GraphicsContext::ScreenIdentifier& si );

    // Return the resolution of specified screen
    // (0,0) is returned if screen is unknown
    virtual void getScreenSettings( const osg::GraphicsContext::ScreenIdentifier& si, osg::GraphicsContext::ScreenSettings & resolution );

    // Return the bits per pixel of specified screen
    // (0) is returned if screen is unknown
    virtual void getScreenColorDepth( const osg::GraphicsContext::ScreenIdentifier& si, unsigned int& dmBitsPerPel );

    // Set the resolution for given screen
    virtual bool setScreenSettings( const osg::GraphicsContext::ScreenIdentifier& si, const osg::GraphicsContext::ScreenSettings & resolution );

    // Enumerates available resolutions
    virtual void enumerateScreenSettings(const osg::GraphicsContext::ScreenIdentifier& screenIdentifier, osg::GraphicsContext::ScreenSettingsList & resolution);

    // Return the screen position and width/height.
    // all zeros returned if screen is unknown
    virtual void getScreenPosition( const osg::GraphicsContext::ScreenIdentifier& si, int& originX, int& originY, unsigned int& width, unsigned int& height );

    // Create a graphics context with given traits
    virtual osg::GraphicsContext* createGraphicsContext( osg::GraphicsContext::Traits* traits );

    // Register a newly created native window along with its application counterpart
    // This is required to maintain a link between Windows messages and the application window object
    // at event processing time
    virtual void registerWindow( HWND hwnd, osgViewer::GraphicsWindowWin32* window );

    // Unregister a window
    // This is called as part of a window being torn down
    virtual void unregisterWindow( HWND hwnd );

    // Get the application window object associated with a native window
    virtual osgViewer::GraphicsWindowWin32* getGraphicsWindowFor( HWND hwnd );

    // Return a valid sample OpenGL Device Context and current rendering context that can be used with wglXYZ extensions
    virtual bool getSampleOpenGLContext( OpenGLContext& context, HDC windowHDC, int windowOriginX, int windowOriginY );

  protected:

    // Display devices present in the system
    typedef std::vector<DISPLAY_DEVICE> DisplayDevices;

    // Map Win32 window handles to GraphicsWindowWin32 instance
    typedef std::pair< HWND, osgViewer::GraphicsWindowWin32* >  WindowHandleEntry;
    typedef std::map<  HWND, osgViewer::GraphicsWindowWin32* >  WindowHandles;

    // Enumerate all display devices and return in passed container
    void enumerateDisplayDevices( DisplayDevices& displayDevices ) const;

    // Get the screen device current mode information
    bool getScreenInformation( const osg::GraphicsContext::ScreenIdentifier& si, DISPLAY_DEVICE& displayDevice, DEVMODE& deviceMode );

    // Change the screen settings (resolution, refresh rate, etc.)
    bool changeScreenSettings( const osg::GraphicsContext::ScreenIdentifier& si, DISPLAY_DEVICE& displayDevice, DEVMODE& deviceMode );

    // Register the window classes used by OSG graphics window instances
    void registerWindowClasses();

    // Unregister the window classes used by OSG graphics window instances
    void unregisterWindowClasses();

    // Data members
    WindowHandles       _activeWindows;                //!< handles to active windows
    bool                _windowClassesRegistered;      //!< true after window classes have been registered

 private:

     // No implementation for these
     Win32WindowingSystem( const Win32WindowingSystem& );
     Win32WindowingSystem& operator=( const Win32WindowingSystem& );
};

///////////////////////////////////////////////////////////////////////////////
//                             Error reporting
//////////////////////////////////////////////////////////////////////////////

static void reportError( const std::string& msg )
{
    OSG_WARN << "Error: " << msg.c_str() << std::endl;
}

static void reportError( const std::string& msg, unsigned int errorCode )
{
    //
    // Some APIs are documented as returning the error in ::GetLastError but apparently do not
    // Skip "Reason" field if the errorCode is still success
    //

    if (errorCode==0)
    {
        reportError(msg);
        return;
    }

    OSG_WARN << "Windows Error #"   << errorCode << ": " << msg.c_str();

    LPVOID lpMsgBuf;

    if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                      NULL,
                      errorCode,
                      0, // Default language
                      (LPTSTR) &lpMsgBuf,
                      0,
                      NULL)!=0)
    {
        OSG_WARN << ". Reason: " << LPTSTR(lpMsgBuf) << std::endl;
        ::LocalFree(lpMsgBuf);
    }
    else
    {
        OSG_WARN << std::endl;
    }
}

static void reportErrorForScreen( const std::string& msg, const osg::GraphicsContext::ScreenIdentifier& si, unsigned int errorCode )
{
    std::ostringstream str;

    str << "[Screen #" << si.screenNum << "] " << msg;
    reportError(str.str(), errorCode);
}

//////////////////////////////////////////////////////////////////////////////
//                       Keyboard key mapping for Win32
//////////////////////////////////////////////////////////////////////////////

class Win32KeyboardMap
{
    public:

        Win32KeyboardMap()
        {
            _keymap[VK_ESCAPE       ] = osgGA::GUIEventAdapter::KEY_Escape;
            _keymap[VK_F1           ] = osgGA::GUIEventAdapter::KEY_F1;
            _keymap[VK_F2           ] = osgGA::GUIEventAdapter::KEY_F2;
            _keymap[VK_F3           ] = osgGA::GUIEventAdapter::KEY_F3;
            _keymap[VK_F4           ] = osgGA::GUIEventAdapter::KEY_F4;
            _keymap[VK_F5           ] = osgGA::GUIEventAdapter::KEY_F5;
            _keymap[VK_F6           ] = osgGA::GUIEventAdapter::KEY_F6;
            _keymap[VK_F7           ] = osgGA::GUIEventAdapter::KEY_F7;
            _keymap[VK_F8           ] = osgGA::GUIEventAdapter::KEY_F8;
            _keymap[VK_F9           ] = osgGA::GUIEventAdapter::KEY_F9;
            _keymap[VK_F10          ] = osgGA::GUIEventAdapter::KEY_F10;
            _keymap[VK_F11          ] = osgGA::GUIEventAdapter::KEY_F11;
            _keymap[VK_F12          ] = osgGA::GUIEventAdapter::KEY_F12;
            _keymap[0xc0            ] = osgGA::GUIEventAdapter::KEY_Backquote;
            _keymap['0'             ] = osgGA::GUIEventAdapter::KEY_0;
            _keymap['1'             ] = osgGA::GUIEventAdapter::KEY_1;
            _keymap['2'             ] = osgGA::GUIEventAdapter::KEY_2;
            _keymap['3'             ] = osgGA::GUIEventAdapter::KEY_3;
            _keymap['4'             ] = osgGA::GUIEventAdapter::KEY_4;
            _keymap['5'             ] = osgGA::GUIEventAdapter::KEY_5;
            _keymap['6'             ] = osgGA::GUIEventAdapter::KEY_6;
            _keymap['7'             ] = osgGA::GUIEventAdapter::KEY_7;
            _keymap['8'             ] = osgGA::GUIEventAdapter::KEY_8;
            _keymap['9'             ] = osgGA::GUIEventAdapter::KEY_9;
            _keymap[0xbd            ] = osgGA::GUIEventAdapter::KEY_Minus;
            _keymap[0xbb            ] = osgGA::GUIEventAdapter::KEY_Equals;
            _keymap[VK_BACK         ] = osgGA::GUIEventAdapter::KEY_BackSpace;
            _keymap[VK_TAB          ] = osgGA::GUIEventAdapter::KEY_Tab;
            _keymap['A'             ] = osgGA::GUIEventAdapter::KEY_A;
            _keymap['B'             ] = osgGA::GUIEventAdapter::KEY_B;
            _keymap['C'             ] = osgGA::GUIEventAdapter::KEY_C;
            _keymap['D'             ] = osgGA::GUIEventAdapter::KEY_D;
            _keymap['E'             ] = osgGA::GUIEventAdapter::KEY_E;
            _keymap['F'             ] = osgGA::GUIEventAdapter::KEY_F;
            _keymap['G'             ] = osgGA::GUIEventAdapter::KEY_G;
            _keymap['H'             ] = osgGA::GUIEventAdapter::KEY_H;
            _keymap['I'             ] = osgGA::GUIEventAdapter::KEY_I;
            _keymap['J'             ] = osgGA::GUIEventAdapter::KEY_J;
            _keymap['K'             ] = osgGA::GUIEventAdapter::KEY_K;
            _keymap['L'             ] = osgGA::GUIEventAdapter::KEY_L;
            _keymap['M'             ] = osgGA::GUIEventAdapter::KEY_M;
            _keymap['N'             ] = osgGA::GUIEventAdapter::KEY_N;
            _keymap['O'             ] = osgGA::GUIEventAdapter::KEY_O;
            _keymap['P'             ] = osgGA::GUIEventAdapter::KEY_P;
            _keymap['Q'             ] = osgGA::GUIEventAdapter::KEY_Q;
            _keymap['R'             ] = osgGA::GUIEventAdapter::KEY_R;
            _keymap['S'             ] = osgGA::GUIEventAdapter::KEY_S;
            _keymap['T'             ] = osgGA::GUIEventAdapter::KEY_T;
            _keymap['U'             ] = osgGA::GUIEventAdapter::KEY_U;
            _keymap['V'             ] = osgGA::GUIEventAdapter::KEY_V;
            _keymap['W'             ] = osgGA::GUIEventAdapter::KEY_W;
            _keymap['X'             ] = osgGA::GUIEventAdapter::KEY_X;
            _keymap['Y'             ] = osgGA::GUIEventAdapter::KEY_Y;
            _keymap['Z'             ] = osgGA::GUIEventAdapter::KEY_Z;
            _keymap[0xdb            ] = osgGA::GUIEventAdapter::KEY_Leftbracket;
            _keymap[0xdd            ] = osgGA::GUIEventAdapter::KEY_Rightbracket;
            _keymap[0xdc            ] = osgGA::GUIEventAdapter::KEY_Backslash;
            _keymap[VK_CAPITAL      ] = osgGA::GUIEventAdapter::KEY_Caps_Lock;
            _keymap[0xba            ] = osgGA::GUIEventAdapter::KEY_Semicolon;
            _keymap[0xde            ] = osgGA::GUIEventAdapter::KEY_Quote;
            _keymap[VK_RETURN       ] = osgGA::GUIEventAdapter::KEY_Return;
            _keymap[VK_LSHIFT       ] = osgGA::GUIEventAdapter::KEY_Shift_L;
            _keymap[0xbc            ] = osgGA::GUIEventAdapter::KEY_Comma;
            _keymap[0xbe            ] = osgGA::GUIEventAdapter::KEY_Period;
            _keymap[0xbf            ] = osgGA::GUIEventAdapter::KEY_Slash;
            _keymap[VK_RSHIFT       ] = osgGA::GUIEventAdapter::KEY_Shift_R;
            _keymap[VK_LCONTROL     ] = osgGA::GUIEventAdapter::KEY_Control_L;
            _keymap[VK_LWIN         ] = osgGA::GUIEventAdapter::KEY_Super_L;
            _keymap[VK_SPACE        ] = osgGA::GUIEventAdapter::KEY_Space;
            _keymap[VK_LMENU        ] = osgGA::GUIEventAdapter::KEY_Alt_L;
            _keymap[VK_RMENU        ] = osgGA::GUIEventAdapter::KEY_Alt_R;
            _keymap[VK_RWIN         ] = osgGA::GUIEventAdapter::KEY_Super_R;
            _keymap[VK_APPS         ] = osgGA::GUIEventAdapter::KEY_Menu;
            _keymap[VK_RCONTROL     ] = osgGA::GUIEventAdapter::KEY_Control_R;
            _keymap[VK_SNAPSHOT     ] = osgGA::GUIEventAdapter::KEY_Print;
            _keymap[VK_SCROLL       ] = osgGA::GUIEventAdapter::KEY_Scroll_Lock;
            _keymap[VK_PAUSE        ] = osgGA::GUIEventAdapter::KEY_Pause;
            _keymap[VK_HOME         ] = osgGA::GUIEventAdapter::KEY_Home;
            _keymap[VK_PRIOR        ] = osgGA::GUIEventAdapter::KEY_Page_Up;
            _keymap[VK_END          ] = osgGA::GUIEventAdapter::KEY_End;
            _keymap[VK_NEXT         ] = osgGA::GUIEventAdapter::KEY_Page_Down;
            _keymap[VK_DELETE       ] = osgGA::GUIEventAdapter::KEY_Delete;
            _keymap[VK_INSERT       ] = osgGA::GUIEventAdapter::KEY_Insert;
            _keymap[VK_LEFT         ] = osgGA::GUIEventAdapter::KEY_Left;
            _keymap[VK_UP           ] = osgGA::GUIEventAdapter::KEY_Up;
            _keymap[VK_RIGHT        ] = osgGA::GUIEventAdapter::KEY_Right;
            _keymap[VK_DOWN         ] = osgGA::GUIEventAdapter::KEY_Down;
            _keymap[VK_NUMLOCK      ] = osgGA::GUIEventAdapter::KEY_Num_Lock;
            _keymap[VK_DIVIDE       ] = osgGA::GUIEventAdapter::KEY_KP_Divide;
            _keymap[VK_MULTIPLY     ] = osgGA::GUIEventAdapter::KEY_KP_Multiply;
            _keymap[VK_SUBTRACT     ] = osgGA::GUIEventAdapter::KEY_KP_Subtract;
            _keymap[VK_ADD          ] = osgGA::GUIEventAdapter::KEY_KP_Add;
            _keymap[VK_NUMPAD7      ] = osgGA::GUIEventAdapter::KEY_KP_Home;
            _keymap[VK_NUMPAD8      ] = osgGA::GUIEventAdapter::KEY_KP_Up;
            _keymap[VK_NUMPAD9      ] = osgGA::GUIEventAdapter::KEY_KP_Page_Up;
            _keymap[VK_NUMPAD4      ] = osgGA::GUIEventAdapter::KEY_KP_Left;
            _keymap[VK_NUMPAD5      ] = osgGA::GUIEventAdapter::KEY_KP_Begin;
            _keymap[VK_NUMPAD6      ] = osgGA::GUIEventAdapter::KEY_KP_Right;
            _keymap[VK_NUMPAD1      ] = osgGA::GUIEventAdapter::KEY_KP_End;
            _keymap[VK_NUMPAD2      ] = osgGA::GUIEventAdapter::KEY_KP_Down;
            _keymap[VK_NUMPAD3      ] = osgGA::GUIEventAdapter::KEY_KP_Page_Down;
            _keymap[VK_NUMPAD0      ] = osgGA::GUIEventAdapter::KEY_KP_Insert;
            _keymap[VK_DECIMAL      ] = osgGA::GUIEventAdapter::KEY_KP_Delete;
            _keymap[VK_CLEAR        ] = osgGA::GUIEventAdapter::KEY_Clear;
        }

        ~Win32KeyboardMap() {}

        int remapKey(int key)
        {
            KeyMap::const_iterator map = _keymap.find(key);
            return map==_keymap.end() ? key : map->second;
        }

    protected:

        typedef std::map<int, int> KeyMap;
        KeyMap _keymap;
};

static Win32KeyboardMap s_win32KeyboardMap;
static int remapWin32Key(int key)
{
    return s_win32KeyboardMap.remapKey(key);
}

//////////////////////////////////////////////////////////////////////////////
//         Window procedure for all GraphicsWindowWin32 instances
//           Dispatches the call to the actual instance
//////////////////////////////////////////////////////////////////////////////

static LRESULT CALLBACK WindowProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    osgViewer::GraphicsWindowWin32* window = Win32WindowingSystem::getInterface()->getGraphicsWindowFor(hwnd);
    return window ? window->handleNativeWindowingEvent(hwnd, uMsg, wParam, lParam) :
                    ::DefWindowProc(hwnd, uMsg, wParam, lParam);
}

//////////////////////////////////////////////////////////////////////////////
//              Win32WindowingSystem::OpenGLContext implementation
//////////////////////////////////////////////////////////////////////////////

Win32WindowingSystem::OpenGLContext::~OpenGLContext()
{
    if (_restorePreviousOnExit && _previousHglrc!=_hglrc && !::wglMakeCurrent(_previousHdc, _previousHglrc))
    {
        reportError("Win32WindowingSystem::OpenGLContext() - Unable to restore current OpenGL rendering context", ::GetLastError());
    }

    _previousHdc   = 0;
    _previousHglrc = 0;

    if (_hglrc)
    {
        ::wglMakeCurrent(_hdc, NULL);
        ::wglDeleteContext(_hglrc);
        _hglrc = 0;
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

bool Win32WindowingSystem::OpenGLContext::makeCurrent( HDC restoreOnHdc, bool restorePreviousOnExit )
{
    if (_hdc==0 || _hglrc==0) return false;

    _previousHglrc = restorePreviousOnExit ? ::wglGetCurrentContext() : 0;
    _previousHdc   = restoreOnHdc;

    if (_hglrc==_previousHglrc) return true;

    if (!::wglMakeCurrent(_hdc, _hglrc))
    {
        reportError("Win32WindowingSystem::OpenGLContext() - Unable to set current OpenGL rendering context", ::GetLastError());
        return false;
    }

    _restorePreviousOnExit = restorePreviousOnExit;

    return true;
}

//////////////////////////////////////////////////////////////////////////////
//              Win32WindowingSystem implementation
//////////////////////////////////////////////////////////////////////////////

std::string Win32WindowingSystem::osgGraphicsWindowWithCursorClass;
std::string Win32WindowingSystem::osgGraphicsWindowWithoutCursorClass;

Win32WindowingSystem::Win32WindowingSystem()
: _windowClassesRegistered(false)
{
  // Detect presence of runtime support for multitouch
    HMODULE hModule = LoadLibrary("user32");
    if (hModule)
    {
        registerTouchWindowFunc = (RegisterTouchWindowFunc *) GetProcAddress( hModule, "RegisterTouchWindow");
        closeTouchInputHandleFunc = (CloseTouchInputHandleFunc *) GetProcAddress( hModule, "CloseTouchInputHandle");
        getTouchInputInfoFunc = (GetTouchInputInfoFunc *)  GetProcAddress( hModule, "GetTouchInputInfo");

        if (!(registerTouchWindowFunc && closeTouchInputHandleFunc && getTouchInputInfoFunc))
        {
            registerTouchWindowFunc = NULL;
            closeTouchInputHandleFunc = NULL;
            getTouchInputInfoFunc = NULL;
            FreeLibrary( hModule);
        }
    }
}

Win32WindowingSystem::~Win32WindowingSystem()
{
    if (osg::Referenced::getDeleteHandler())
    {
        osg::Referenced::getDeleteHandler()->setNumFramesToRetainObjects(0);
        osg::Referenced::getDeleteHandler()->flushAll();
    }

    unregisterWindowClasses();
}

void Win32WindowingSystem::enumerateDisplayDevices( DisplayDevices& displayDevices ) const
{
    for (unsigned int deviceNum=0;; ++deviceNum)
    {
        DISPLAY_DEVICE displayDevice;
        displayDevice.cb = sizeof(displayDevice);

        if (!::EnumDisplayDevices(NULL, deviceNum, &displayDevice, 0)) break;

        // Do not track devices used for remote access (Terminal Services pseudo-displays, etc.)
        if (displayDevice.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER) continue;

        // Only return display devices that are attached to the desktop
        if (!(displayDevice.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP)) continue;

        displayDevices.push_back(displayDevice);
    }
}

void Win32WindowingSystem::registerWindowClasses()
{
    if (_windowClassesRegistered) return;

    //
    // Register the window classes used by OSG GraphicsWindowWin32 instances
    //

    std::ostringstream str;
    str << "OSG Graphics Window for Win32 [" << ::GetCurrentProcessId() << "]";

    osgGraphicsWindowWithCursorClass    = str.str() + "{ with cursor }";
    osgGraphicsWindowWithoutCursorClass = str.str() + "{ without cursor }";

    WNDCLASSEX wc;

    HINSTANCE hinst = ::GetModuleHandle(NULL);

    //
    // First class: class for OSG Graphics Window with a cursor enabled
    //

    wc.cbSize        = sizeof(wc);
    wc.style         = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc   = WindowProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hinst;
    wc.hIcon         = ::LoadIcon(hinst, "OSG_ICON");
    wc.hCursor       = ::LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszMenuName  = 0;
    wc.lpszClassName = osgGraphicsWindowWithCursorClass.c_str();
    wc.hIconSm       = NULL;

    if (::RegisterClassEx(&wc)==0)
    {
        unsigned int lastError = ::GetLastError();
        if (lastError!=ERROR_CLASS_ALREADY_EXISTS)
        {
            reportError("Win32WindowingSystem::registerWindowClasses() - Unable to register first window class", lastError);
            return;
        }
    }

    //
    // Second class: class for OSG Graphics Window without a cursor
    //

    wc.hCursor       = NULL;
    wc.lpszClassName = osgGraphicsWindowWithoutCursorClass.c_str();

    if (::RegisterClassEx(&wc)==0)
    {
        unsigned int lastError = ::GetLastError();
        if (lastError!=ERROR_CLASS_ALREADY_EXISTS)
        {
            reportError("Win32WindowingSystem::registerWindowClasses() - Unable to register second window class", lastError);
            return;
        }
    }

    _windowClassesRegistered = true;
}

void Win32WindowingSystem::unregisterWindowClasses()
{
    if (_windowClassesRegistered)
    {
        ::UnregisterClass(osgGraphicsWindowWithCursorClass.c_str(),    ::GetModuleHandle(NULL));
        ::UnregisterClass(osgGraphicsWindowWithoutCursorClass.c_str(), ::GetModuleHandle(NULL));
        _windowClassesRegistered = false;
    }
}

bool Win32WindowingSystem::getSampleOpenGLContext( OpenGLContext& context, HDC windowHDC, int windowOriginX, int windowOriginY )
{
    context.set(0, 0, 0);

    registerWindowClasses();

    HWND hwnd = ::CreateWindowEx(WS_EX_OVERLAPPEDWINDOW,
                                 osgGraphicsWindowWithoutCursorClass.c_str(),
                                 NULL,
                                 WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_DISABLED,
                                 windowOriginX,
                                 windowOriginY,
                                 1,
                                 1,
                                 NULL,
                                 NULL,
                                 ::GetModuleHandle(NULL),
                                 NULL);
    if (hwnd==0)
    {
        reportError("Win32WindowingSystem::getSampleOpenGLContext() - Unable to create window", ::GetLastError());
        return false;
    }

    //
    // Set the pixel format of the window
    //

    PIXELFORMATDESCRIPTOR pixelFormat =
    {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL,
        PFD_TYPE_RGBA,
        24,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0,
        24,
        0,
        0,
        PFD_MAIN_PLANE,
        0,
        0, 0, 0
    };

    HDC hdc = ::GetDC(hwnd);
    if (hdc==0)
    {
        reportError("Win32WindowingSystem::getSampleOpenGLContext() - Unable to get window device context", ::GetLastError());
        ::DestroyWindow(hwnd);
        return false;
    }

    int pixelFormatIndex = ::ChoosePixelFormat(hdc, &pixelFormat);
    if (pixelFormatIndex==0)
    {
        reportError("Win32WindowingSystem::getSampleOpenGLContext() - Unable to choose pixel format", ::GetLastError());
        ::ReleaseDC(hwnd, hdc);
        ::DestroyWindow(hwnd);
        return false;
    }

    if (!::SetPixelFormat(hdc, pixelFormatIndex, &pixelFormat))
    {
        reportError("Win32WindowingSystem::getSampleOpenGLContext() - Unable to set pixel format", ::GetLastError());
        ::ReleaseDC(hwnd, hdc);
        ::DestroyWindow(hwnd);
        return false;
    }

    HGLRC hglrc = ::wglCreateContext(hdc);
    if (hglrc==0)
    {
        reportError("Win32WindowingSystem::getSampleOpenGLContext() - Unable to create an OpenGL rendering context", ::GetLastError());
        ::ReleaseDC(hwnd, hdc);
        ::DestroyWindow(hwnd);
        return false;
    }

    context.set(hwnd, hdc, hglrc);

    if (!context.makeCurrent(windowHDC, true)) return false;

    return true;
}

unsigned int Win32WindowingSystem::getNumScreens( const osg::GraphicsContext::ScreenIdentifier& si )
{
    return si.displayNum==0 ? ::GetSystemMetrics(SM_CMONITORS) : 0;
}

bool Win32WindowingSystem::getScreenInformation( const osg::GraphicsContext::ScreenIdentifier& si, DISPLAY_DEVICE& displayDevice, DEVMODE& deviceMode )
{
    if (si.displayNum>0)
    {
        OSG_WARN << "Win32WindowingSystem::getScreenInformation() - The screen identifier on the Win32 platform must always use display number 0. Value received was " << si.displayNum << std::endl;
        return false;
    }

    DisplayDevices displayDevices;
    enumerateDisplayDevices(displayDevices);

    if (si.screenNum>=static_cast<int>(displayDevices.size()))
    {
        OSG_WARN << "Win32WindowingSystem::getScreenInformation() - Cannot get information for screen " << si.screenNum << " because it does not exist." << std::endl;
        return false;
    }

    displayDevice = displayDevices[si.screenNum];

    deviceMode.dmSize        = sizeof(deviceMode);
    deviceMode.dmDriverExtra = 0;

    if (!::EnumDisplaySettings(displayDevice.DeviceName, ENUM_CURRENT_SETTINGS, &deviceMode))
    {
        std::ostringstream str;
        str << "Win32WindowingSystem::getScreenInformation() - Unable to query information for screen number " << si.screenNum;
        reportError(str.str(), ::GetLastError());
        return false;
    }

    return true;
}

void Win32WindowingSystem::getScreenSettings( const osg::GraphicsContext::ScreenIdentifier& si, osg::GraphicsContext::ScreenSettings & resolution )
{
    DISPLAY_DEVICE displayDevice;
    DEVMODE        deviceMode;

    if (!getScreenInformation(si, displayDevice, deviceMode))
        deviceMode.dmFields = 0;        // Set the fields to 0 so that it says 'nothing'.

    // Get resolution
    if ((deviceMode.dmFields & (DM_PELSWIDTH | DM_PELSHEIGHT)) != 0) {
        resolution.width  = deviceMode.dmPelsWidth;
        resolution.height = deviceMode.dmPelsHeight;
    } else {
        resolution.width  = 0;
        resolution.height = 0;
    }

    // Get refersh rate
    if ((deviceMode.dmFields & DM_DISPLAYFREQUENCY) != 0) {
        resolution.refreshRate = deviceMode.dmDisplayFrequency;
        if (resolution.refreshRate == 0 || resolution.refreshRate == 1) {
            // Windows specific: 0 and 1 represent the hhardware's default refresh rate.
            // If someone knows how to get this refresh rate (in Hz)...
            OSG_NOTICE << "Win32WindowingSystem::getScreenSettings() is not fully implemented (cannot retreive the hardware's default refresh rate)."<<std::endl;
            resolution.refreshRate = 0;
        }
    } else
        resolution.refreshRate = 0;

    // Get bits per pixel for color buffer
    if ((deviceMode.dmFields & DM_BITSPERPEL) != 0)
        resolution.colorDepth = deviceMode.dmBitsPerPel;
    else
        resolution.colorDepth = 0;
}

void Win32WindowingSystem::getScreenColorDepth( const osg::GraphicsContext::ScreenIdentifier& si, unsigned int& dmBitsPerPel )
{
    DISPLAY_DEVICE displayDevice;
    DEVMODE        deviceMode;

    if (getScreenInformation(si, displayDevice, deviceMode))
    {
        dmBitsPerPel = deviceMode.dmBitsPerPel;
    }
    else
    {
        dmBitsPerPel  = 0;
    }
}

bool Win32WindowingSystem::changeScreenSettings( const osg::GraphicsContext::ScreenIdentifier& si, DISPLAY_DEVICE& displayDevice, DEVMODE& deviceMode )
{
    //
    // Start by testing if the change would be successful (without applying it)
    //

    unsigned int result = ::ChangeDisplaySettingsEx(displayDevice.DeviceName, &deviceMode, NULL, CDS_TEST, NULL);
    if (result==DISP_CHANGE_SUCCESSFUL)
    {
        result = ::ChangeDisplaySettingsEx(displayDevice.DeviceName, &deviceMode, NULL, 0, NULL);
        if (result==DISP_CHANGE_SUCCESSFUL) return true;
    }

    std::string msg = "Win32WindowingSystem::changeScreenSettings() - Unable to change the screen settings.";

    switch( result )
    {
        case DISP_CHANGE_BADMODE     : msg += " The specified graphics mode is not supported."; break;
        case DISP_CHANGE_FAILED      : msg += " The display driver failed the specified graphics mode."; break;
        case DISP_CHANGE_RESTART     : msg += " The computer must be restarted for the graphics mode to work."; break;
        default : break;
    }

    reportErrorForScreen(msg, si, result);
    return false;
}

bool Win32WindowingSystem::setScreenSettings( const osg::GraphicsContext::ScreenIdentifier& si, const osg::GraphicsContext::ScreenSettings & resolution )
{
    DISPLAY_DEVICE displayDevice;
    DEVMODE        deviceMode;

    if (!getScreenInformation(si, displayDevice, deviceMode)) return false;

    deviceMode.dmFields = 0;
    // Set resolution
    if (resolution.width>0 && resolution.height>0) {
        deviceMode.dmFields    |= DM_PELSWIDTH | DM_PELSHEIGHT;
        deviceMode.dmPelsWidth  = static_cast<DWORD>(resolution.width);
        deviceMode.dmPelsHeight = static_cast<DWORD>(resolution.height);
    }
    // Set refersh rate
    if (resolution.refreshRate>0) {
        deviceMode.dmFields           |= DM_DISPLAYFREQUENCY;
        deviceMode.dmDisplayFrequency  = static_cast<DWORD>(resolution.refreshRate);
    }
    // Set bits per pixel for color buffer
    if (resolution.colorDepth>0) {
        deviceMode.dmFields     |= DM_BITSPERPEL;
        deviceMode.dmBitsPerPel  = static_cast<DWORD>(resolution.colorDepth);
    }

    return changeScreenSettings(si, displayDevice, deviceMode);
}

void Win32WindowingSystem::enumerateScreenSettings(const osg::GraphicsContext::ScreenIdentifier& si, osg::GraphicsContext::ScreenSettingsList & resolutionList) {
    resolutionList.clear();

    if (si.displayNum>0)
    {
        OSG_WARN << "Win32WindowingSystem::enumerateScreenSettings() - The screen identifier on the Win32 platform must always use display number 0. Value received was " << si.displayNum << std::endl;
        return;
    }

    DisplayDevices displayDevices;
    enumerateDisplayDevices(displayDevices);

    if (si.screenNum>=static_cast<int>(displayDevices.size()))
    {
        OSG_WARN << "Win32WindowingSystem::enumerateScreenSettings() - Cannot get information for screen " << si.screenNum << " because it does not exist." << std::endl;
        return;
    }

    DISPLAY_DEVICE displayDevice = displayDevices[si.screenNum];

    // Do the enumeration
    DEVMODE deviceMode;
    static const unsigned int MAX_RESOLUTIONS = 4046;        // Upper limit to avoid infinite (= very long) loop.
    for (unsigned int i=0; i<MAX_RESOLUTIONS; ++i)
    {
        if (!::EnumDisplaySettings(displayDevice.DeviceName, i, &deviceMode))
            break;
        deviceMode.dmSize        = sizeof(deviceMode);
        deviceMode.dmDriverExtra = 0;
        resolutionList.push_back(osg::GraphicsContext::ScreenSettings(deviceMode.dmPelsWidth, deviceMode.dmPelsHeight, deviceMode.dmDisplayFrequency, deviceMode.dmBitsPerPel));
    }
}

void Win32WindowingSystem::getScreenPosition( const osg::GraphicsContext::ScreenIdentifier& si, int& originX, int& originY, unsigned int& width, unsigned int& height )
{
    DISPLAY_DEVICE displayDevice;
    DEVMODE        deviceMode;

    if (getScreenInformation(si, displayDevice, deviceMode))
    {
        originX = deviceMode.dmPosition.x;
        originY = deviceMode.dmPosition.y;
        width   = deviceMode.dmPelsWidth;
        height  = deviceMode.dmPelsHeight;
    }
    else
    {
        originX = 0;
        originY = 0;
        width   = 0;
        height  = 0;
    }
}

osg::GraphicsContext* Win32WindowingSystem::createGraphicsContext( osg::GraphicsContext::Traits* traits )
{
    if (traits->pbuffer)
    {
        osg::ref_ptr<osgViewer::PixelBufferWin32> pbuffer = new PixelBufferWin32(traits);
        if (pbuffer->valid()) return pbuffer.release();
        else return 0;
    }
    else
    {
        registerWindowClasses();

        osg::ref_ptr<osgViewer::GraphicsWindowWin32> window = new GraphicsWindowWin32(traits);
        if (window->valid()) return window.release();
        else return 0;
    }
}

void Win32WindowingSystem::registerWindow( HWND hwnd, osgViewer::GraphicsWindowWin32* window )
{
    if (hwnd) _activeWindows.insert(WindowHandleEntry(hwnd, window));
}

//
// Unregister a window
// This is called as part of a window being torn down
//

void Win32WindowingSystem::unregisterWindow( HWND hwnd )
{
    if (hwnd) _activeWindows.erase(hwnd);
}

//
// Get the application window object associated with a native window
//

osgViewer::GraphicsWindowWin32* Win32WindowingSystem::getGraphicsWindowFor( HWND hwnd )
{
    WindowHandles::const_iterator entry = _activeWindows.find(hwnd);
    return entry==_activeWindows.end() ? 0 : entry->second;
}

//////////////////////////////////////////////////////////////////////////////
//                    GraphicsWindowWin32 implementation
//////////////////////////////////////////////////////////////////////////////

GraphicsWindowWin32::GraphicsWindowWin32( osg::GraphicsContext::Traits* traits )
: _currentCursor(0),
  _windowProcedure(0),
  _timeOfLastCheckEvents(-1.0),
  _screenOriginX(0),
  _screenOriginY(0),
  _screenWidth(0),
  _screenHeight(0),
  _windowOriginXToRealize(0),
  _windowOriginYToRealize(0),
  _windowWidthToRealize(0),
  _windowHeightToRealize(0),
  _initialized(false),
  _valid(false),
  _realized(false),
  _ownsWindow(true),
  _closeWindow(false),
  _destroyWindow(false),
  _destroying(false),
  _mouseCursor(InheritCursor),
  _appMouseCursor(LeftArrowCursor),
  _applyWorkaroundForMultimonitorMultithreadNVidiaWin32Issues( false )
{
    _traits = traits;
    if (_traits->useCursor) setCursor(LeftArrowCursor);
    else setCursor(NoCursor);

    init();

    if (valid())
    {
        setState( new osg::State );
        getState()->setGraphicsContext(this);

        if (_traits.valid() && _traits->sharedContext.valid())
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

GraphicsWindowWin32::~GraphicsWindowWin32()
{
    close();
    destroyWindow();
}

void GraphicsWindowWin32::init()
{
    if (_initialized) return;

    // getEventQueue()->setCurrentEventState(osgGA::GUIEventAdapter::getAccumulatedEventState().get());

    WindowData *windowData = _traits.get() ? dynamic_cast<WindowData*>(_traits->inheritedWindowData.get()) : 0;
    HWND windowHandle = windowData ? windowData->_hwnd : 0;

    _ownsWindow    = windowHandle==0;
    _closeWindow   = false;
    _destroyWindow = false;
    _destroying    = false;

    _initialized = _ownsWindow ? createWindow() : setWindow(windowHandle);
    _valid       = _initialized;
    
    // make sure the event queue has the correct window rectangle size and input range
    getEventQueue()->syncWindowRectangleWithGraphcisContext();

    // 2008/10/03
    // Few days ago NVidia released WHQL certified drivers ver 178.13.
    // These drivers (as well as former beta ver 177.92) were free from the bug described below.
    // So it looks like its high time to make the workaround inactive by default.
    // If you happen to still use earlier drivers and have problems consider changing to new ones or
    // activate OSG_MULTIMONITOR_MULTITHREAD_WIN32_NVIDIA_WORKAROUND macro def through CMake advanced vars.
#ifdef OSG_MULTIMONITOR_MULTITHREAD_WIN32_NVIDIA_WORKAROUND

    // 2008/05/12
    // Workaround for Bugs in NVidia drivers for windows XP / multithreaded / dualview / multicore CPU
    // affects GeForce 6x00, 7x00, 8x00 boards (others were not tested) driver versions 174.xx - 175.xx
    // pre 174.xx had other issues so reverting is not an option (statitistics, fbo)
    // drivers release 175.16 is the latest currently available
    //
    // When using OpenGL in threaded app ( main thread sets up context / renderer thread draws using it )
    // first wglMakeCurrent seems to not work right and screw OpenGL context driver data:
    // 1: succesive drawing shows a number of artifacts in TriangleStrips and TriangleFans
    // 2: weird behaviour of FramBufferObjects (glGenFramebuffer generates already generated ids ...)
    // Looks like repeating wglMakeCurrent call fixes all these issues
    // wglMakeCurrent call can impact performance so I try to minimize number of
    // wglMakeCurrent calls by checking current HDC and GL context
    // and repeat wglMakeCurrent only when they change for current thread

    _applyWorkaroundForMultimonitorMultithreadNVidiaWin32Issues = true;
#endif

    const char* str = getenv("OSG_WIN32_NV_MULTIMON_MULTITHREAD_WORKAROUND");
    if (str)
    {
        _applyWorkaroundForMultimonitorMultithreadNVidiaWin32Issues = (strcmp(str, "on")==0 || strcmp(str, "ON")==0 || strcmp(str, "On")==0 );
    }
}

bool GraphicsWindowWin32::createWindow()
{
    unsigned int extendedStyle;
    unsigned int windowStyle;

    if (!determineWindowPositionAndStyle(_traits->screenNum,
                                         _traits->x,
                                         _traits->y,
                                         _traits->width,
                                         _traits->height,
                                         _traits->windowDecoration,
                                         _windowOriginXToRealize,
                                         _windowOriginYToRealize,
                                         _windowWidthToRealize,
                                         _windowHeightToRealize,
                                         windowStyle,
                                         extendedStyle))
    {
        reportError("GraphicsWindowWin32::createWindow() - Unable to determine the window position and style");
        return false;
    }

    _hwnd = ::CreateWindowEx(extendedStyle,
                             _traits->useCursor ? Win32WindowingSystem::osgGraphicsWindowWithCursorClass.c_str() :
                                                  Win32WindowingSystem::osgGraphicsWindowWithoutCursorClass.c_str(),
                             _traits->windowName.c_str(),
                             windowStyle,
                             _windowOriginXToRealize,
                             _windowOriginYToRealize,
                             _windowWidthToRealize,
                             _windowHeightToRealize,
                             NULL,
                             NULL,
                             ::GetModuleHandle(NULL),
                             NULL);
    if (_hwnd==0)
    {
        reportErrorForScreen("GraphicsWindowWin32::createWindow() - Unable to create window", _traits->screenNum, ::GetLastError());
        return false;
    }

    _hdc = ::GetDC(_hwnd);
    if (_hdc==0)
    {
        reportErrorForScreen("GraphicsWindowWin32::createWindow() - Unable to get window device context", _traits->screenNum, ::GetLastError());
        destroyWindow();
        _hwnd = 0;
        return false;
    }

    //
    // Set the pixel format according to traits specified
    //

    if (!setPixelFormat())
    {
        ::ReleaseDC(_hwnd, _hdc);
        _hdc  = 0;
        destroyWindow();
        return false;
    }

    //
    // Create the OpenGL rendering context associated with this window
    //

    _hglrc = createContextImplementation();
    if (_hglrc==0)
    {
        reportErrorForScreen("GraphicsWindowWin32::createWindow() - Unable to create OpenGL rendering context", _traits->screenNum, ::GetLastError());
        ::ReleaseDC(_hwnd, _hdc);
        _hdc  = 0;
        destroyWindow();
        return false;
    }

    Win32WindowingSystem::getInterface()->registerWindow(_hwnd, this);

    if (registerTouchWindowFunc)
        (*registerTouchWindowFunc)( _hwnd, 0);
    return true;
}

bool GraphicsWindowWin32::setWindow( HWND handle )
{
    if (_initialized)
    {
        reportErrorForScreen("GraphicsWindowWin32::setWindow() - Window already created; it cannot be changed", _traits->screenNum, ::GetLastError());
        return false;
    }

    if (handle==0)
    {
        reportErrorForScreen("GraphicsWindowWin32::setWindow() - Invalid window handle passed", _traits->screenNum, ::GetLastError());
        return false;
    }

    _hwnd = handle;
    if (_hwnd==0)
    {
        reportErrorForScreen("GraphicsWindowWin32::setWindow() - Unable to retrieve native window handle", _traits->screenNum, ::GetLastError());
        return false;
    }

    _hdc = ::GetDC(_hwnd);
    if (_hdc==0)
    {
        reportErrorForScreen("GraphicsWindowWin32::setWindow() - Unable to get window device context", _traits->screenNum, ::GetLastError());
        _hwnd = 0;
        return false;
    }

    //
    // Check if we must set the pixel format of the inherited window
    //

    if (!setPixelFormat())
    {
        reportErrorForScreen("GraphicsWindowWin32::setWindow() - Unable to set the inherited window pixel format", _traits->screenNum, ::GetLastError());
        ::ReleaseDC(_hwnd, _hdc);
        _hdc  = 0;
        _hwnd = 0;
        return false;
    }

    _hglrc = createContextImplementation();
    if (_hglrc==0)
    {
        reportErrorForScreen("GraphicsWindowWin32::setWindow() - Unable to create OpenGL rendering context", _traits->screenNum, ::GetLastError());
        ::ReleaseDC(_hwnd, _hdc);
        _hdc  = 0;
        _hwnd = 0;
        return false;
    }

    WindowData *windowData = _traits.get() ? dynamic_cast<WindowData*>(_traits->inheritedWindowData.get()) : 0;

    if (!windowData || windowData->_installEventHandler)
    {
        if (!registerWindowProcedure())
        {
            ::wglDeleteContext(_hglrc);
            _hglrc = 0;
            ::ReleaseDC(_hwnd, _hdc);
            _hdc  = 0;
            _hwnd = 0;
            return false;
        }
    }

    Win32WindowingSystem::getInterface()->registerWindow(_hwnd, this);

    _initialized = true;
    _valid       = true;

    return true;
}

void GraphicsWindowWin32::destroyWindow( bool deleteNativeWindow )
{
    if (_destroying) return;
    _destroying = true;

    if (_graphicsThread && _graphicsThread->isRunning())
    {
        // find all the viewers that might own use this graphics context
        osg::GraphicsContext::Cameras cameras = getCameras();
        for(osg::GraphicsContext::Cameras::iterator it=cameras.begin(); it!=cameras.end(); ++it)
        {
            osgViewer::View* view = dynamic_cast<osgViewer::View*>((*it)->getView());
            osgViewer::ViewerBase* viewerBase = view ? view->getViewerBase() : 0;
            if (viewerBase && viewerBase->areThreadsRunning())
            {
                viewerBase->stopThreading();
            }
        }
    }

    if (_hdc)
    {
        releaseContext();

        if (_hglrc)
        {
            ::wglDeleteContext(_hglrc);
            _hglrc = 0;
        }

        ::ReleaseDC(_hwnd, _hdc);
        _hdc = 0;
    }

    (void)unregisterWindowProcedure();

    if (_hwnd)
    {
        Win32WindowingSystem::getInterface()->unregisterWindow(_hwnd);
        if (_ownsWindow && deleteNativeWindow) ::DestroyWindow(_hwnd);
        _hwnd = 0;
    }

    _initialized = false;
    _realized    = false;
    _valid       = false;
    _destroying  = false;
}

void GraphicsWindowWin32::registerWindow()
{
  Win32WindowingSystem::getInterface()->registerWindow(_hwnd, this);
}

void GraphicsWindowWin32::unregisterWindow()
{
  Win32WindowingSystem::getInterface()->unregisterWindow(_hwnd);
}

bool GraphicsWindowWin32::registerWindowProcedure()
{
    ::SetLastError(0);
    _windowProcedure = (WNDPROC)::SetWindowLongPtr(_hwnd, GWLP_WNDPROC, LONG_PTR(WindowProc));
    unsigned int error = ::GetLastError();

    if (_windowProcedure==0 && error)
    {
        reportErrorForScreen("GraphicsWindowWin32::registerWindowProcedure() - Unable to register window procedure", _traits->screenNum, error);
        return false;
    }

    return true;
}

bool GraphicsWindowWin32::unregisterWindowProcedure()
{
    if (_windowProcedure==0 || _hwnd==0) return true;

    ::SetLastError(0);
    WNDPROC wndProc = (WNDPROC)::SetWindowLongPtr(_hwnd, GWLP_WNDPROC, LONG_PTR(_windowProcedure));
    unsigned int error = ::GetLastError();

    if (wndProc==0 && error)
    {
        reportErrorForScreen("GraphicsWindowWin32::unregisterWindowProcedure() - Unable to unregister window procedure", _traits->screenNum, error);
        return false;
    }

    _windowProcedure = 0;

    return true;
}

bool GraphicsWindowWin32::determineWindowPositionAndStyle( unsigned int  screenNum,
                                                           int           clientAreaX,
                                                           int           clientAreaY,
                                                           unsigned int  clientAreaWidth,
                                                           unsigned int  clientAreaHeight,
                                                           bool          decorated,
                                                           int&          x,
                                                           int&          y,
                                                           unsigned int& w,
                                                           unsigned int& h,
                                                           unsigned int& style,
                                                           unsigned int& extendedStyle )
{
    if (_traits==0) return false;

    //
    // Query the screen position and size
    //

    osg::GraphicsContext::ScreenIdentifier screenId(screenNum);
    Win32WindowingSystem* windowManager = Win32WindowingSystem::getInterface();

    windowManager->getScreenPosition(screenId, _screenOriginX, _screenOriginY, _screenWidth, _screenHeight);
    if (_screenWidth==0 || _screenHeight==0) return false;

    x = clientAreaX + _screenOriginX;
    y = clientAreaY + _screenOriginY;
    w = clientAreaWidth;
    h = clientAreaHeight;

    style = WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

    extendedStyle = 0;

    if (decorated)
    {
        style |= WS_CAPTION     |
                 WS_SYSMENU     |
                 WS_MINIMIZEBOX |
                 WS_MAXIMIZEBOX;

        if (_traits->supportsResize) style |= WS_SIZEBOX;

        extendedStyle = WS_EX_APPWINDOW           |
                        WS_EX_OVERLAPPEDWINDOW |
                        WS_EX_ACCEPTFILES      |
                        WS_EX_LTRREADING;

        RECT corners;

        corners.left   = x;
        corners.top    = y;
        corners.right  = x + w - 1;
        corners.bottom = y + h - 1;

        //
        // Determine the location of the window corners in order to have
        // a client area of the requested size
        //

        if (!::AdjustWindowRectEx(&corners, style, FALSE, extendedStyle))
        {
            reportErrorForScreen("GraphicsWindowWin32::determineWindowPositionAndStyle() - Unable to adjust window rectangle", _traits->screenNum, ::GetLastError());
            return false;
        }

        x = corners.left;
        y = corners.top;
        w = corners.right  - corners.left + 1;
        h = corners.bottom - corners.top  + 1;
    }

    return true;
}

static void PreparePixelFormatSpecifications( const osg::GraphicsContext::Traits& traits,
                                              WGLIntegerAttributes&               attributes,
                                              bool                                allowSwapExchangeARB )
{
    attributes.begin();

    attributes.enable(WGL_DRAW_TO_WINDOW_ARB);
    attributes.enable(WGL_SUPPORT_OPENGL_ARB);

    attributes.set(WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB);
    attributes.set(WGL_PIXEL_TYPE_ARB,   WGL_TYPE_RGBA_ARB);

    attributes.set(WGL_COLOR_BITS_ARB,   traits.red + traits.green + traits.blue);
    attributes.set(WGL_RED_BITS_ARB,     traits.red);
    attributes.set(WGL_GREEN_BITS_ARB,   traits.green);
    attributes.set(WGL_BLUE_BITS_ARB,    traits.blue);
    attributes.set(WGL_DEPTH_BITS_ARB,   traits.depth);

    if (traits.doubleBuffer)
    {
        attributes.enable(WGL_DOUBLE_BUFFER_ARB);

        switch ( traits.swapMethod )
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
                if ( allowSwapExchangeARB )
                    attributes.set(WGL_SWAP_METHOD_ARB, WGL_SWAP_EXCHANGE_ARB);
                break;
        }
    }

    if (traits.alpha)         attributes.set(WGL_ALPHA_BITS_ARB,     traits.alpha);
    if (traits.stencil)       attributes.set(WGL_STENCIL_BITS_ARB,   traits.stencil);
    if (traits.sampleBuffers) attributes.set(WGL_SAMPLE_BUFFERS_ARB, traits.sampleBuffers);
    if (traits.samples)       attributes.set(WGL_SAMPLES_ARB,        traits.samples);

    if (traits.quadBufferStereo) attributes.enable(WGL_STEREO_ARB);

    attributes.end();
}

static int ChooseMatchingPixelFormat( HDC hdc, int screenNum, const WGLIntegerAttributes& formatSpecifications ,osg::GraphicsContext::Traits* _traits)
{
    //
    // Access the entry point for the wglChoosePixelFormatARB function
    //

    WGLChoosePixelFormatARB wglChoosePixelFormatARB = (WGLChoosePixelFormatARB)wglGetProcAddress("wglChoosePixelFormatARB");
    if (wglChoosePixelFormatARB==0)
    {
        // = openGLContext.getTraits()
        reportErrorForScreen("ChooseMatchingPixelFormat() - wglChoosePixelFormatARB extension not found, trying GDI", screenNum, ::GetLastError());
        PIXELFORMATDESCRIPTOR pixelFormat = {
            sizeof(PIXELFORMATDESCRIPTOR),  //  size of this pfd
            1,                     // version number
            PFD_DRAW_TO_WINDOW |   // support window
            PFD_SUPPORT_OPENGL |   // support OpenGL
            (_traits->doubleBuffer ? PFD_DOUBLEBUFFER : NULL) |      // double buffered ?
            (_traits->swapMethod ==  osg::DisplaySettings::SWAP_COPY ? PFD_SWAP_COPY : NULL) |
            (_traits->swapMethod ==  osg::DisplaySettings::SWAP_EXCHANGE ? PFD_SWAP_EXCHANGE : NULL),
            PFD_TYPE_RGBA,         // RGBA type
            _traits->red + _traits->green + _traits->blue,                // color depth
            _traits->red ,0, _traits->green ,0, _traits->blue, 0,          // shift bits ignored
            _traits->alpha,          // alpha buffer ?
            0,                     // shift bit ignored
            0,                     // no accumulation buffer
            0, 0, 0, 0,            // accum bits ignored
            _traits->depth,          // 32 or 16 bit z-buffer ?
            _traits->stencil,        // stencil buffer ?
            0,                     // no auxiliary buffer
            PFD_MAIN_PLANE,        // main layer
            0,                     // reserved
            0, 0, 0                // layer masks ignored
        };
        int pixelFormatIndex = ::ChoosePixelFormat(hdc, &pixelFormat);
        if (pixelFormatIndex == 0)
        {
            reportErrorForScreen("ChooseMatchingPixelFormat() - GDI ChoosePixelFormat Failed.", screenNum, ::GetLastError());
            return -1;
        }

        ::DescribePixelFormat(hdc, pixelFormatIndex ,sizeof(PIXELFORMATDESCRIPTOR),&pixelFormat);
        if (((pixelFormat.dwFlags & PFD_GENERIC_FORMAT) != 0)  && ((pixelFormat.dwFlags & PFD_GENERIC_ACCELERATED) == 0))
        {
            OSG_WARN << "Rendering in software: pixelFormatIndex " << pixelFormatIndex << std::endl;
        }
        return pixelFormatIndex;
    }

    int pixelFormatIndex = 0;
    unsigned int numMatchingPixelFormats = 0;

    if (!wglChoosePixelFormatARB(hdc,
                                 formatSpecifications.get(),
                                 NULL,
                                 1,
                                 &pixelFormatIndex,
                                 &numMatchingPixelFormats))
    {
        reportErrorForScreen("ChooseMatchingPixelFormat() - Unable to choose the requested pixel format", screenNum, ::GetLastError());
        return -1;
    }

    return numMatchingPixelFormats==0 ? -1 : pixelFormatIndex;
}

bool GraphicsWindowWin32::setPixelFormat()
{
    Win32WindowingSystem::OpenGLContext openGLContext;
    if (!Win32WindowingSystem::getInterface()->getSampleOpenGLContext(openGLContext, _hdc, _screenOriginX, _screenOriginY)) return false;

    //
    // Build the specifications of the requested pixel format
    //

    WGLIntegerAttributes formatSpecs;
    ::PreparePixelFormatSpecifications(*_traits, formatSpecs, true);

    //
    // Choose the closest matching pixel format from the specified traits
    //

    int pixelFormatIndex = ::ChooseMatchingPixelFormat(openGLContext.deviceContext(), _traits->screenNum, formatSpecs,_traits.get());

    if (pixelFormatIndex<0)
    {
            unsigned int bpp;
            Win32WindowingSystem::getInterface()->getScreenColorDepth(*_traits.get(), bpp);
            if (bpp < 32) {
                OSG_INFO    << "GraphicsWindowWin32::setPixelFormat() - Display setting is not 32 bit colors, "
                                        << bpp
                                        << " bits per pixel on screen #"
                                        << _traits->screenNum
                                        << std::endl;

                _traits->red = bpp / 4; //integer devide, determine minimum number of bits we will accept
                _traits->green = bpp / 4;
                _traits->blue = bpp / 4;
                ::PreparePixelFormatSpecifications(*_traits, formatSpecs, true);// try again with WGL_SWAP_METHOD_ARB
                pixelFormatIndex = ::ChooseMatchingPixelFormat(openGLContext.deviceContext(), _traits->screenNum, formatSpecs,_traits.get());
            }
    }
    if (pixelFormatIndex<0)
    {
        ::PreparePixelFormatSpecifications(*_traits, formatSpecs, false);
        pixelFormatIndex = ::ChooseMatchingPixelFormat(openGLContext.deviceContext(), _traits->screenNum, formatSpecs,_traits.get());
        if (pixelFormatIndex<0)
        {
            reportErrorForScreen("GraphicsWindowWin32::setPixelFormat() - No matching pixel format found based on traits specified", _traits->screenNum, 0);
            return false;
        }

        OSG_INFO << "GraphicsWindowWin32::setPixelFormat() - Found a matching pixel format but without the WGL_SWAP_METHOD_ARB specification for screen #"
                               << _traits->screenNum
                               << std::endl;
    }

    //
    // Set the pixel format found
    //

    PIXELFORMATDESCRIPTOR pfd;
    ::memset(&pfd, 0, sizeof(pfd));
    pfd.nSize    = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;

    if (!::SetPixelFormat(_hdc, pixelFormatIndex, &pfd))
    {
        reportErrorForScreen("GraphicsWindowWin32::setPixelFormat() - Unable to set pixel format", _traits->screenNum, ::GetLastError());
        return false;
    }

    return true;
}

HGLRC GraphicsWindowWin32::createContextImplementation()
{
    HGLRC context( NULL );

    if( OSG_GL3_FEATURES )
    {
        OSG_NOTIFY( osg::INFO ) << "GL3: Attempting to create OpenGL3 context." << std::endl;
        OSG_NOTIFY( osg::INFO ) << "GL3: version: " << _traits->glContextVersion << std::endl;
        OSG_NOTIFY( osg::INFO ) << "GL3: context flags: " << _traits->glContextFlags << std::endl;
        OSG_NOTIFY( osg::INFO ) << "GL3: profile: " << _traits->glContextProfileMask << std::endl;

        Win32WindowingSystem::OpenGLContext openGLContext;
        if( !Win32WindowingSystem::getInterface()->getSampleOpenGLContext( openGLContext, _hdc, _screenOriginX, _screenOriginY ) )
        {
            reportErrorForScreen( "GL3: Can't create sample context.",
                _traits->screenNum, ::GetLastError() );
        }
        else
        {
            PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB =
                ( PFNWGLCREATECONTEXTATTRIBSARBPROC ) wglGetProcAddress( "wglCreateContextAttribsARB" );
            if( wglCreateContextAttribsARB==0 )
            {
                reportErrorForScreen( "GL3: wglCreateContextAttribsARB not available.",
                    _traits->screenNum, ::GetLastError() );
            }
            else
            {
                unsigned int idx( 0 );
                int attribs[ 16 ];

                unsigned int major = 1, minor = 0;
                if( !_traits->getContextVersion(major, minor) || major<3 )
                {
                    OSG_NOTIFY( osg::WARN ) << "GL3: Non-GL3 version number: " << _traits->glContextVersion << std::endl;
                }

                attribs[ idx++ ] = WGL_CONTEXT_MAJOR_VERSION_ARB;
                attribs[ idx++ ] = major;
                attribs[ idx++ ] = WGL_CONTEXT_MINOR_VERSION_ARB;
                attribs[ idx++ ] = minor;
                if( _traits->glContextFlags != 0 )
                {
                    attribs[ idx++ ] = WGL_CONTEXT_FLAGS_ARB;
                    attribs[ idx++ ] = _traits->glContextFlags;
                }
                if( _traits->glContextProfileMask != 0 )
                {
                    attribs[ idx++ ] = WGL_CONTEXT_PROFILE_MASK_ARB;
                    attribs[ idx++ ] = _traits->glContextProfileMask;
                }
                attribs[ idx++ ] = 0;

                context = wglCreateContextAttribsARB( _hdc, 0, attribs );
                if( context == NULL )
                {
                    reportErrorForScreen( "GL3: wglCreateContextAttribsARB returned NULL.",
                        _traits->screenNum, ::GetLastError() );
                }
                else
                {
                    OSG_NOTIFY( osg::INFO ) << "GL3: context created successfully." << std::endl;
                }
            }
        }
    }

    // TBD insert GL ES 2 suppurt, if required for Win32.

    // If platform context creation fails for any reason,
    // we'll create a standard context. This means you could
    // build OSG for GL3, have the context creation fail
    // (because you have the wrong driver), and end up with
    // a GL3 context. Something else will likely fail down
    // the line, as the GL3-built OSG will assume GL3 features
    // are present.
    //
    // This is also the typical path for GL 1/2 context creation.
    if( context == NULL )
        context = ::wglCreateContext(_hdc);

    return( context );
}

bool GraphicsWindowWin32::setWindowDecorationImplementation( bool decorated )
{
    unsigned int windowStyle;
    unsigned int extendedStyle;

    //
    // Determine position and size of window with/without decorations to retain the size specified in traits
    //

    int x, y;
    unsigned int w, h;

    if (!determineWindowPositionAndStyle(_traits->screenNum,
                                 _traits->x,
                     _traits->y,
                     _traits->width,
                     _traits->height,
                     decorated,
                     x,
                     y,
                     w,
                     h,
                     windowStyle,
                     extendedStyle))
    {
        reportErrorForScreen("GraphicsWindowWin32::setWindowDecoration() - Unable to determine the window position and style", _traits->screenNum, 0);
        return false;
    }

    //
    // Change the window style
    //

    ::SetLastError(0);
    unsigned int result = ::SetWindowLong(_hwnd, GWL_STYLE, windowStyle);
    unsigned int error  = ::GetLastError();
    if (result==0 && error)
    {
        reportErrorForScreen("GraphicsWindowWin32::setWindowDecoration() - Unable to set window style", _traits->screenNum, error);
        return false;
    }

    //
    // Change the window extended style
    //

    ::SetLastError(0);
    result = ::SetWindowLong(_hwnd, GWL_EXSTYLE, extendedStyle);
    error  = ::GetLastError();
    if (result==0 && error)
    {
        reportErrorForScreen("GraphicsWindowWin32::setWindowDecoration() - Unable to set window extented style", _traits->screenNum, error);
        return false;
    }

    //
    // Change the window position and size and realize the style changes
    //

    if (!::SetWindowPos(_hwnd, HWND_TOP, x, y, w, h, SWP_FRAMECHANGED | SWP_NOZORDER | SWP_SHOWWINDOW))
    {
        reportErrorForScreen("GraphicsWindowWin32::setWindowDecoration() - Unable to set new window position and size", _traits->screenNum, ::GetLastError());
        return false;
    }

    //
    // Force a repaint of the desktop
    //

    ::InvalidateRect(NULL, NULL, TRUE);

    return true;
}

bool GraphicsWindowWin32::realizeImplementation()
{
    if (_realized) return true;

    if (!_initialized)
    {
        init();
        if (!_initialized) return false;
    }

    if (_traits.valid() && (_traits->sharedContext.valid() || _traits->vsync || _traits->swapGroupEnabled))
    {
        // make context current so we can test capabilities and set up context sharing
        struct RestoreContext
        {
            RestoreContext()
            {
                _hdc = wglGetCurrentDC();
                _hglrc = wglGetCurrentContext();
            }
            ~RestoreContext()
            {
                wglMakeCurrent(_hdc,_hglrc);
            }
        protected:
            HDC      _hdc;
            HGLRC    _hglrc;
        } restoreContext;

        _realized = true;
        bool result = makeCurrent();
        _realized = false;

        if (!result)
        {
            return false;
        }

        // set up sharing of contexts if required
        GraphicsHandleWin32* graphicsHandleWin32 = dynamic_cast<GraphicsHandleWin32*>(_traits->sharedContext.get());
        if (graphicsHandleWin32)
        {
            if (!wglShareLists(graphicsHandleWin32->getWGLContext(), getWGLContext()))
            {
                reportErrorForScreen("GraphicsWindowWin32::realizeImplementation() - Unable to share OpenGL context", _traits->screenNum, ::GetLastError());
                return false;
            }
        }

        // if vysnc should be on then enable it.
        if (_traits->vsync)
        {
            setSyncToVBlank(_traits->vsync);
        }

        // If the swap group is active then enable it.
        if (_traits->swapGroupEnabled)
        {
            setSwapGroup(_traits->swapGroupEnabled, _traits->swapGroup, _traits->swapBarrier);
        }
    }

    if (_ownsWindow)
    {
        //
        // Bring the window on top of other ones (including the taskbar if it covers it completely)
        //
        // NOTE: To cover the taskbar with a window that does not completely cover it, the HWND_TOPMOST
        // Z-order must be used in the code below instead of HWND_TOP.
        // @todo: This should be controlled through a flag in the traits (topMostWindow)
        //

        if (!::SetWindowPos(_hwnd,
                            HWND_TOP,
                            _windowOriginXToRealize,
                            _windowOriginYToRealize,
                            _windowWidthToRealize,
                            _windowHeightToRealize,
                            SWP_SHOWWINDOW))
        {
            reportErrorForScreen("GraphicsWindowWin32::realizeImplementation() - Unable to show window", _traits->screenNum, ::GetLastError());
            return false;
        }

        if (!::UpdateWindow(_hwnd))
        {
            reportErrorForScreen("GraphicsWindowWin32::realizeImplementation() - Unable to update window", _traits->screenNum, ::GetLastError());
            return false;
        }
    }

    _realized = true;

    // make sure the event queue has the correct window rectangle size and input range
    getEventQueue()->syncWindowRectangleWithGraphcisContext();

    return true;
}

bool GraphicsWindowWin32::makeCurrentImplementation()
{
    if (!_realized)
    {
        reportErrorForScreen("GraphicsWindowWin32::makeCurrentImplementation() - Window not realized; cannot do makeCurrent.", _traits->screenNum, 0);
        return false;
    }

    if( _applyWorkaroundForMultimonitorMultithreadNVidiaWin32Issues )
    {
        if( ::wglGetCurrentDC() != _hdc ||
            ::wglGetCurrentContext() != _hglrc )
        {
            if (!::wglMakeCurrent(_hdc, _hglrc))
            {
                reportErrorForScreen("GraphicsWindowWin32::makeCurrentImplementation() - Unable to set current OpenGL rendering context", _traits->screenNum, ::GetLastError());
                return false;
            }
        }
    }

    if (!::wglMakeCurrent(_hdc, _hglrc))
    {
        reportErrorForScreen("GraphicsWindowWin32::makeCurrentImplementation() - Unable to set current OpenGL rendering context", _traits->screenNum, ::GetLastError());
        return false;
    }

    return true;
}

bool GraphicsWindowWin32::releaseContextImplementation()
{
    if (!::wglMakeCurrent(_hdc, NULL))
    {
        reportErrorForScreen("GraphicsWindowWin32::releaseContextImplementation() - Unable to release current OpenGL rendering context", _traits->screenNum, ::GetLastError());
        return false;
    }

    return true;
}

void GraphicsWindowWin32::closeImplementation()
{
    destroyWindow();

    _initialized = false;
    _valid       = false;
    _realized    = false;
}

void GraphicsWindowWin32::swapBuffersImplementation()
{
    if (!_realized) return;
    if (!::SwapBuffers(_hdc) && ::GetLastError() != 0)
    {
        reportErrorForScreen("GraphicsWindowWin32::swapBuffersImplementation() - Unable to swap display buffers", _traits->screenNum, ::GetLastError());
    }
}

bool GraphicsWindowWin32::checkEvents()
{
    if (!_realized) return false;

    MSG msg;
    while (::PeekMessage(&msg, _hwnd, 0, 0, PM_REMOVE))
    {
        ::TranslateMessage(&msg);
        ::DispatchMessage(&msg);
    }

    if (_closeWindow)
    {
        _closeWindow = false;
        close();
    }

    if (_destroyWindow)
    {
        _destroyWindow = false;
        destroyWindow(false);
    }
           
    return !(getEventQueue()->empty());
}

void GraphicsWindowWin32::grabFocus()
{
    if (!::SetForegroundWindow(_hwnd))
    {
        OSG_WARN << "Warning: GraphicsWindowWin32::grabFocus() - Failed grabbing the focus" << std::endl;
    }
}

void GraphicsWindowWin32::grabFocusIfPointerInWindow()
{
    POINT mousePos;
    if (!::GetCursorPos(&mousePos))
    {
        reportErrorForScreen("GraphicsWindowWin32::grabFocusIfPointerInWindow() - Unable to get cursor position", _traits->screenNum, ::GetLastError());
        return;
    }

    RECT windowRect;
    if (!::GetWindowRect(_hwnd, &windowRect))
    {
        reportErrorForScreen("GraphicsWindowWin32::grabFocusIfPointerInWindow() - Unable to get window position", _traits->screenNum, ::GetLastError());
        return;
    }

    if (mousePos.x>=windowRect.left && mousePos.x<=windowRect.right &&
        mousePos.y>=windowRect.top  && mousePos.y<=windowRect.bottom)
    {
        grabFocus();
    }
}

void GraphicsWindowWin32::requestWarpPointer( float x, float y )
{
    if (!_realized)
    {
        OSG_INFO<<"GraphicsWindowWin32::requestWarpPointer() - Window not realized; cannot warp pointer, screenNum="<< _traits->screenNum<<std::endl;
        return;
    }

#if 0
    RECT windowRect;
    if (!::GetWindowRect(_hwnd, &windowRect))
    {
        reportErrorForScreen("GraphicsWindowWin32::requestWarpPointer() - Unable to get window rectangle", _traits->screenNum, ::GetLastError());
        return;
    }

    if (!::SetCursorPos(windowRect.left + x, windowRect.top + y))
    {
        reportErrorForScreen("GraphicsWindowWin32::requestWarpPointer() - Unable to set cursor position", _traits->screenNum, ::GetLastError());
        return;
    }
#else
    // MIKEC: NEW CODE
    POINT pt;
    pt.x = (LONG)x;
    pt.y = (LONG)y;

    // convert point in client area coordinates to screen coordinates
    if (!::ClientToScreen(_hwnd, &pt))
    {
        reportErrorForScreen("GraphicsWindowWin32::requestWarpPointer() - Unable to convert cursor position to screen coordinates", _traits->screenNum, ::GetLastError());
    }
    if (!::SetCursorPos(pt.x, pt.y))
    {
        reportErrorForScreen("GraphicsWindowWin32::requestWarpPointer() - Unable to set cursor position", _traits->screenNum, ::GetLastError());
        return;
    }
#endif

    getEventQueue()->mouseWarped(x,y);
}

bool GraphicsWindowWin32::setWindowRectangleImplementation(int x, int y, int width, int height)
{
    unsigned int windowStyle;
    unsigned int extendedStyle;

    //
    // Determine position and size of window with/without decorations to retain the size specified in traits
    //

    int wx, wy;
    unsigned int ww, wh;

    if (!determineWindowPositionAndStyle(_traits->screenNum,
                                         x,
                                         y,
                                         width,
                                         height,
                                         _traits->windowDecoration,
                                         wx,
                                         wy,
                                         ww,
                                         wh,
                                         windowStyle,
                                         extendedStyle))
    {
        reportErrorForScreen("GraphicsWindowWin32::setWindowRectangleImplementation() - Unable to determine the window position and style", _traits->screenNum, 0);
        return false;
    }

    if (!::SetWindowPos(_hwnd, HWND_TOP, wx, wy, ww, wh, SWP_SHOWWINDOW | SWP_FRAMECHANGED))
    {
        reportErrorForScreen("GraphicsWindowWin32::setWindowRectangleImplementation() - Unable to set new window position and size", _traits->screenNum, ::GetLastError());
        return false;
    }
    return true;
}

void GraphicsWindowWin32::setWindowName( const std::string & name )
{
    _traits->windowName = name;
    SetWindowText(_hwnd, name.c_str());
}

void GraphicsWindowWin32::useCursor( bool cursorOn )
{
    if (_traits.valid())
        _traits->useCursor = cursorOn;

    // note, we are using setCursorImpl to set the cursor, so we can use
    // _appMouseCursor to cache the current mouse-cursor
    setCursorImpl(cursorOn ? _appMouseCursor : NoCursor);
}

void GraphicsWindowWin32::setCursor( MouseCursor mouseCursor )
{
    _appMouseCursor = mouseCursor;
    setCursorImpl(mouseCursor);
}

void GraphicsWindowWin32::setCursorImpl( MouseCursor mouseCursor )
{
    if (_mouseCursor != mouseCursor)
    {
        _mouseCursor = mouseCursor;
        HCURSOR newCursor = getOrCreateCursor( mouseCursor);
        if (newCursor == _currentCursor) return;

        _currentCursor = newCursor;
        _traits->useCursor = (_currentCursor != NULL) && (_mouseCursor != NoCursor);

        if (_mouseCursor != InheritCursor)
            ::SetCursor(_currentCursor);
    }
}

HCURSOR GraphicsWindowWin32::getOrCreateCursor(MouseCursor mouseCursor)
{
    std::map<MouseCursor,HCURSOR>::iterator i = _mouseCursorMap.find(mouseCursor);
    if (i != _mouseCursorMap.end()) return i->second;

    switch (mouseCursor) {
    case NoCursor:
        _mouseCursorMap[mouseCursor] = NULL;
    break;
    case RightArrowCursor:
        _mouseCursorMap[mouseCursor] = LoadCursor( NULL, IDC_ARROW);
        break;
    case LeftArrowCursor:
        _mouseCursorMap[mouseCursor] = LoadCursor( NULL, IDC_ARROW);
        break;
    case InfoCursor:
        _mouseCursorMap[mouseCursor] = LoadCursor( NULL, IDC_SIZEALL);
        break;
    case DestroyCursor:
        _mouseCursorMap[mouseCursor] = LoadCursor( NULL, IDC_NO );
        break;
    case HelpCursor:
        _mouseCursorMap[mouseCursor] = LoadCursor( NULL, IDC_HELP );
        break;
    case CycleCursor:
        _mouseCursorMap[mouseCursor] = LoadCursor( NULL, IDC_NO );
        break;
    case SprayCursor:
        _mouseCursorMap[mouseCursor] = LoadCursor( NULL, IDC_SIZEALL );
        break;
    case WaitCursor:
        _mouseCursorMap[mouseCursor] = LoadCursor( NULL, IDC_WAIT);
        break;
    case TextCursor:
        _mouseCursorMap[mouseCursor] = LoadCursor( NULL, IDC_IBEAM );
        break;
    case CrosshairCursor:
        _mouseCursorMap[mouseCursor] = LoadCursor( NULL, IDC_CROSS );
        break;
    case UpDownCursor:
        _mouseCursorMap[mouseCursor] = LoadCursor( NULL, IDC_SIZENS );
        break;
    case LeftRightCursor:
        _mouseCursorMap[mouseCursor] = LoadCursor( NULL, IDC_SIZEWE );
        break;
    case TopSideCursor:
        _mouseCursorMap[mouseCursor] = LoadCursor( NULL, IDC_UPARROW );
        break;
    case BottomSideCursor:
        _mouseCursorMap[mouseCursor] = LoadCursor( NULL, IDC_UPARROW );
        break;
    case LeftSideCursor:
        _mouseCursorMap[mouseCursor] = LoadCursor( NULL, IDC_SIZEWE);
        break;
    case RightSideCursor:
        _mouseCursorMap[mouseCursor] = LoadCursor( NULL, IDC_SIZEWE );
        break;
    case TopLeftCorner:
        _mouseCursorMap[mouseCursor] = LoadCursor( NULL, IDC_SIZENWSE );
        break;
    case TopRightCorner:
        _mouseCursorMap[mouseCursor] = LoadCursor( NULL, IDC_SIZENESW );
        break;
    case BottomRightCorner:
        _mouseCursorMap[mouseCursor] = LoadCursor( NULL, IDC_SIZENWSE );
        break;
    case BottomLeftCorner:
        _mouseCursorMap[mouseCursor] = LoadCursor( NULL, IDC_SIZENESW );
        break;
    case HandCursor:
        _mouseCursorMap[mouseCursor] = LoadCursor( NULL, IDC_HAND );
        break;
    default:
        break;
    }

    return _mouseCursorMap[mouseCursor];
}

void GraphicsWindowWin32::setSwapGroup(bool on, GLuint group, GLuint barrier)
{
    if (_traits.valid())
    {
        _traits->swapGroupEnabled = on;
        _traits->swapGroup        = group;
        _traits->swapBarrier      = barrier;
    }

    typedef BOOL (GL_APIENTRY *PFNWGLJOINSWAPGROUPNVPROC) (HDC hDC, GLuint group);
    PFNWGLJOINSWAPGROUPNVPROC wglJoinSwapGroupNV = (PFNWGLJOINSWAPGROUPNVPROC)wglGetProcAddress( "wglJoinSwapGroupNV" );

    typedef BOOL (GL_APIENTRY *PFNWGLBINDSWAPBARRIERNVPROC) (GLuint group, GLuint barrier);
    PFNWGLBINDSWAPBARRIERNVPROC wglBindSwapBarrierNV = (PFNWGLBINDSWAPBARRIERNVPROC)wglGetProcAddress( "wglBindSwapBarrierNV" );

    if ((!wglJoinSwapGroupNV) || (!wglBindSwapBarrierNV))
    {
        OSG_INFO << "GraphicsWindowWin32::wglJoinSwapGroupNV(bool, GLuint, GLuint) not supported" << std::endl;
        return;
    }

    int swapGroup = (on ? group : 0);
    BOOL resultJoin = wglJoinSwapGroupNV(_hdc, swapGroup);
    OSG_INFO << "GraphicsWindowWin32::wglJoinSwapGroupNV (" << swapGroup << ") returned " << resultJoin << std::endl;

    int swapBarrier = (on ? barrier : 0);
    BOOL resultBind = wglBindSwapBarrierNV(swapGroup, swapBarrier);
    OSG_INFO << "GraphicsWindowWin32::wglBindSwapBarrierNV (" << swapGroup << ", " << swapBarrier << ") returned " << resultBind << std::endl;
}

void GraphicsWindowWin32::setSyncToVBlank( bool on )
{
    if (_traits.valid())
    {
        _traits->vsync = on;
    }

//#if 0
    // we ought to properly check if the extension is listed as supported rather than just
    // if the function pointer resolves through wglGetProcAddress, but in practice everything
    // supports this extension
    typedef BOOL (GL_APIENTRY *PFNWGLSWAPINTERVALFARPROC)( int );
    PFNWGLSWAPINTERVALFARPROC wglSwapIntervalEXT = 0;

    wglSwapIntervalEXT = (PFNWGLSWAPINTERVALFARPROC)wglGetProcAddress( "wglSwapIntervalEXT" );
    if( wglSwapIntervalEXT )
    {
        int swapInterval = (on ? 1 : 0);
        wglSwapIntervalEXT(swapInterval);
        OSG_INFO << "GraphicsWindowWin32::setSyncToVBlank " << (on ? "on" : "off") << std::endl;
    }
    else
    {
        OSG_INFO << "GraphicsWindowWin32::setSyncToVBlank(bool) not supported" << std::endl;
    }
//#else
//    OSG_INFO << "GraphicsWindowWin32::setSyncToVBlank(bool) not yet implemented."<< std::endl;
//#endif
}

void GraphicsWindowWin32::adaptKey( WPARAM wParam, LPARAM lParam, int& keySymbol, unsigned int& modifierMask, int& unmodifiedKeySymbol)
{
    modifierMask = 0;

    bool rightSide = (lParam & 0x01000000)!=0;
    int virtualKey = ::MapVirtualKeyEx((lParam>>16) & 0xff, 3, ::GetKeyboardLayout(0));

    BYTE keyState[256];

    if (virtualKey==0 || !::GetKeyboardState(keyState))
    {
        keySymbol = 0;
        return;
    }

    switch (virtualKey)
    {
        //////////////////
        case VK_LSHIFT   :
        //////////////////

        modifierMask |= osgGA::GUIEventAdapter::MODKEY_LEFT_SHIFT;
            break;

        //////////////////
        case VK_RSHIFT   :
        //////////////////

        modifierMask |= osgGA::GUIEventAdapter::MODKEY_RIGHT_SHIFT;
            break;

        //////////////////
        case VK_CONTROL  :
        case VK_LCONTROL :
        //////////////////

            virtualKey    = rightSide ? VK_RCONTROL : VK_LCONTROL;
            modifierMask |= rightSide ? osgGA::GUIEventAdapter::MODKEY_RIGHT_CTRL : osgGA::GUIEventAdapter::MODKEY_LEFT_CTRL;
            break;

        //////////////////
        case VK_MENU     :
        case VK_LMENU    :
        //////////////////

            virtualKey    = rightSide ? VK_RMENU : VK_LMENU;
            modifierMask |= rightSide ? osgGA::GUIEventAdapter::MODKEY_RIGHT_ALT : osgGA::GUIEventAdapter::MODKEY_LEFT_ALT;
            break;

        //////////////////
        default          :
        //////////////////

            virtualKey = wParam;
            break;
    }

    if (keyState[VK_CAPITAL] & 0x01) modifierMask |= osgGA::GUIEventAdapter::MODKEY_CAPS_LOCK;
    if (keyState[VK_NUMLOCK] & 0x01) modifierMask |= osgGA::GUIEventAdapter::MODKEY_NUM_LOCK;

    keySymbol = remapWin32Key(virtualKey);

    if (keySymbol==osgGA::GUIEventAdapter::KEY_Return && rightSide)
    {
        keySymbol = osgGA::GUIEventAdapter::KEY_KP_Enter;
    }

    unmodifiedKeySymbol = keySymbol;

    if ((keySymbol & 0xff00)==0)
    {
        char asciiKey[2];
        int numChars = ::ToAscii(wParam, (lParam>>16)&0xff, keyState, reinterpret_cast<WORD*>(asciiKey), 0);
        if (numChars>0) keySymbol = asciiKey[0];
    }
}

void GraphicsWindowWin32::transformMouseXY( float& x, float& y )
{
    if (getEventQueue()->getUseFixedMouseInputRange())
    {
        osgGA::GUIEventAdapter* eventState = getEventQueue()->getCurrentEventState();

        x = eventState->getXmin() + (eventState->getXmax()-eventState->getXmin())*x/float(_traits->width);
        y = eventState->getYmin() + (eventState->getYmax()-eventState->getYmin())*y/float(_traits->height);
    }
}

LRESULT GraphicsWindowWin32::handleNativeWindowingEvent( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    //!@todo adapt windows event time to osgGA event queue time for better resolution
    double eventTime  = getEventQueue()->getTime();
    double resizeTime = eventTime;
    _timeOfLastCheckEvents = eventTime;

    switch(uMsg)
    {
        // Wojtek Lewandowski 2010-09-28:
        // All web docs on Windows Aero and OpenGL compatibiltiy
        // suggest WM_ERASEBKGND should be handled with non NULL value return.
        // This sugesstion may be irrelevant for our window class
        // as default brush pattern is not set so erase flag is forwarded to WM_PAINT
        // and gets ignored when WM_PAINT is handled.
        // But it will certainly be safer and not make things worse
        // if we handle this message to be sure everything is done as suggested.
        case WM_ERASEBKGND :
            return TRUE;
            break;

        /////////////////
        case WM_PAINT   :
        /////////////////

            if (_ownsWindow)
            {
                PAINTSTRUCT paint;
                ::BeginPaint(hwnd, &paint);
                ::EndPaint(hwnd, &paint);
                requestRedraw();
            }
            break;

        ///////////////////
        case WM_MOUSEMOVE :
        ///////////////////

            {
                float mx = GET_X_LPARAM(lParam);
                float my = GET_Y_LPARAM(lParam);
                transformMouseXY(mx, my);
                getEventQueue()->mouseMotion(mx, my, eventTime);
            }
            break;

        /////////////////////
        case WM_LBUTTONDOWN :
        case WM_MBUTTONDOWN :
        case WM_RBUTTONDOWN :
        /////////////////////

            {
                ::SetCapture(hwnd);

                int button;

                if (uMsg==WM_LBUTTONDOWN)      button = 1;
                else if (uMsg==WM_MBUTTONDOWN) button = 2;
                else button = 3;

                _capturedMouseButtons.insert(button);

                float mx = GET_X_LPARAM(lParam);
                float my = GET_Y_LPARAM(lParam);
                transformMouseXY(mx, my);
                getEventQueue()->mouseButtonPress(mx, my, button, eventTime);
            }
            break;

        /////////////////////
        case WM_LBUTTONUP   :
        case WM_MBUTTONUP   :
        case WM_RBUTTONUP   :
        /////////////////////

            {
                int button;

                if (uMsg==WM_LBUTTONUP)      button = 1;
                else if (uMsg==WM_MBUTTONUP) button = 2;
                else button = 3;

                _capturedMouseButtons.erase(button);

                if(_capturedMouseButtons.empty())
                  ::ReleaseCapture();

                float mx = GET_X_LPARAM(lParam);
                float my = GET_Y_LPARAM(lParam);
                transformMouseXY(mx, my);
                getEventQueue()->mouseButtonRelease(mx, my, button, eventTime);
            }
            break;

        ///////////////////////
        case WM_LBUTTONDBLCLK :
        case WM_MBUTTONDBLCLK :
        case WM_RBUTTONDBLCLK :
        ///////////////////////

            {
                ::SetCapture(hwnd);

                int button;

                if (uMsg==WM_LBUTTONDBLCLK)            button = 1;
                else if (uMsg==WM_MBUTTONDBLCLK)    button = 2;
                else button = 3;

                _capturedMouseButtons.insert(button);

                float mx = GET_X_LPARAM(lParam);
                float my = GET_Y_LPARAM(lParam);
                transformMouseXY(mx, my);
                getEventQueue()->mouseDoubleButtonPress(mx, my, button, eventTime);
            }
            break;

        ////////////////////
        case WM_MOUSEWHEEL :
        ////////////////////

            getEventQueue()->mouseScroll(GET_WHEEL_DELTA_WPARAM(wParam)<0 ? osgGA::GUIEventAdapter::SCROLL_DOWN :
                                                                            osgGA::GUIEventAdapter::SCROLL_UP,
                                         eventTime);
            break;

        /////////////////
        case WM_MOVE    :
        case WM_SIZE    :
        /////////////////

            {
                POINT origin;
                origin.x = 0;
                origin.y = 0;

                ::ClientToScreen(hwnd, &origin);

                int windowX = origin.x - _screenOriginX;
                int windowY = origin.y - _screenOriginY;
                resizeTime  = eventTime;

                RECT clientRect;
                ::GetClientRect(hwnd, &clientRect);

                int windowWidth = (clientRect.right == 0) ? 1 : clientRect.right ;
                int windowHeight = (clientRect.bottom == 0) ? 1 : clientRect.bottom;;

                // send resize event if window position or size was changed
                if (windowX!=_traits->x || windowY!=_traits->y || windowWidth!=_traits->width || windowHeight!=_traits->height)
                {
                    resized(windowX, windowY, windowWidth, windowHeight);
                    getEventQueue()->windowResize(windowX, windowY, windowWidth, windowHeight, resizeTime);

                    // request redraw if window size was changed
                    if (windowWidth!=_traits->width || windowHeight!=_traits->height)
                        requestRedraw();
                }
            }
            break;

        ////////////////////
        case WM_KEYDOWN    :
        case WM_SYSKEYDOWN :
        ////////////////////

            {
                int keySymbol = 0;
                int unmodifiedKeySymbol = 0;
                unsigned int modifierMask = 0;
                adaptKey(wParam, lParam, keySymbol, modifierMask, unmodifiedKeySymbol);
                _keyMap[std::make_pair(keySymbol,unmodifiedKeySymbol)] = true;
                //getEventQueue()->getCurrentEventState()->setModKeyMask(modifierMask);
                getEventQueue()->keyPress(keySymbol, eventTime, unmodifiedKeySymbol);
            }
            break;

        //////////////////
        case WM_KEYUP    :
        case WM_SYSKEYUP :
        //////////////////

            {
                int keySymbol = 0;
                int unmodifiedKeySymbol = 0;
                unsigned int modifierMask = 0;
                adaptKey(wParam, lParam, keySymbol, modifierMask, unmodifiedKeySymbol);
                _keyMap[std::make_pair(keySymbol, unmodifiedKeySymbol)] = false;
                //getEventQueue()->getCurrentEventState()->setModKeyMask(modifierMask);
                getEventQueue()->keyRelease(keySymbol, eventTime, unmodifiedKeySymbol);
            }
            break;

        ///////////////////
        case WM_SETCURSOR :
        ///////////////////
            //The cursor is only modified in response to the WM_SETCURSOR message if the mouse cursor isn't set to
            //InheritCursor.  InheritCursor lets the user manage the cursor externally.
            if (_mouseCursor != InheritCursor)
            {
                if (_traits->useCursor)
                    ::SetCursor( _currentCursor);
                else
                    ::SetCursor(NULL);
                return TRUE;
            }
            break;

        ///////////////////
        case WM_SETFOCUS :
        ///////////////////
            // Check keys and send a message if the key is pressed when the
            // focus comes back to the window.
            // I don't really like this hard-coded loop, but the key codes
            // (VK_* constants) seem to go from 0x08 to 0xFE so it should be
            // ok. See winuser.h for the key codes.
            for (unsigned int i = 0x08; i < 0xFF; i++)
            {
                // Wojciech Lewandowski: 2011/09/12
                // Skip CONTROL | MENU | SHIFT tests because we are polling exact left or right keys
                // above return press for both right and left so we may end up with incosistent
                // modifier mask if we report left control & right control while only right was pressed
                LONG rightSideCode = 0;
                switch( i )
                {
                    case VK_CONTROL:
                    case VK_SHIFT:
                    case VK_MENU:
                        continue;

                    case VK_RCONTROL:
                    case VK_RSHIFT:
                    case VK_RMENU:
                        rightSideCode = 0x01000000;
                }
                if ((::GetAsyncKeyState(i) & 0x8000) != 0)
                {
                    // Compute lParam because subsequent adaptKey will rely on correct lParam
                    UINT scanCode = ::MapVirtualKeyEx( i, 0, ::GetKeyboardLayout(0));
                    // Set Extended Key bit + Scan Code + 30 bit to indicate key was set before sending message
                    // See Windows SDK help on WM_KEYDOWN for explanation
                    LONG lParam = rightSideCode | ( ( scanCode & 0xFF ) << 16 ) | (1 << 30);
                    ::SendMessage(hwnd, WM_KEYDOWN, i, lParam );
                }
            }
            break;

        ///////////////////
        case WM_KILLFOCUS :
        ///////////////////

            // Release all keys that were pressed when the window lost focus.
            for (std::map<std::pair<int, int>, bool>::iterator key = _keyMap.begin();
                 key != _keyMap.end(); ++key)
            {
                if (key->second)
                {
                    getEventQueue()->keyRelease(key->first.first, key->first.second);
                    key->second = false;
                }
            }

            _capturedMouseButtons.clear();

            break;

        ///////////////////
        case WM_NCHITTEST :
        ///////////////////
            {
                LONG_PTR result = _windowProcedure==0 ? ::DefWindowProc(hwnd, uMsg, wParam, lParam) :
                                                        ::CallWindowProc(_windowProcedure, hwnd, uMsg, wParam, lParam);

                switch(result)
                {
                case HTLEFT:
                case HTRIGHT:
                    setCursorImpl(LeftRightCursor);
                    break;
                case HTTOP:
                case HTBOTTOM:
                    setCursorImpl(UpDownCursor);
                    break;
                case HTTOPLEFT:
                    setCursorImpl(TopLeftCorner);
                    break;
                case HTTOPRIGHT:
                    setCursorImpl(TopRightCorner);
                    break;
                case HTBOTTOMLEFT:
                    setCursorImpl(BottomLeftCorner);
                    break;
                case HTBOTTOMRIGHT:
                case HTGROWBOX:
                    setCursorImpl(BottomRightCorner);
                    break;
                case HTSYSMENU:
                case HTCAPTION:
                case HTMAXBUTTON:
                case HTMINBUTTON:
                case HTCLOSE:
                case HTHELP:
                   setCursorImpl(LeftArrowCursor);
                   break;

                default:
                    if (_traits->useCursor && _appMouseCursor != InheritCursor)
                        setCursorImpl(_appMouseCursor);
                    break;
                }
                return result;
            }
            break;

        /////////////////
        case WM_CLOSE   :
        /////////////////

            getEventQueue()->closeWindow(eventTime);
            break;

        /////////////////
        case WM_DESTROY :
        /////////////////

            _destroyWindow = true;
            if (_ownsWindow)
            {
                ::PostQuitMessage(0);
            }
            break;

        //////////////
        case WM_QUIT :
        //////////////

            _closeWindow = true;
            return wParam;

        //////////////
        case WM_TOUCH:
        /////////////
            {
                unsigned int numInputs = (unsigned int) wParam;
                TOUCHINPUT* ti = new TOUCHINPUT[numInputs];
                osg::ref_ptr<osgGA::GUIEventAdapter> osg_event(NULL);
                if(getTouchInputInfoFunc && (*getTouchInputInfoFunc)((HTOUCHINPUT)lParam, numInputs, ti, sizeof(TOUCHINPUT)))
                {
                    // For each contact, dispatch the message to the appropriate message handler.
                    for(unsigned int i=0; i< numInputs; ++i)
                    {
                        if(ti[i].dwFlags & TOUCHEVENTF_DOWN)
                        {
                            if (!osg_event) {
                                osg_event = getEventQueue()->touchBegan( ti[i].dwID, osgGA::GUIEventAdapter::TOUCH_BEGAN, ti[i].x / 100 , ti[i].y/100);
                            } else {
                                osg_event->addTouchPoint( ti[i].dwID, osgGA::GUIEventAdapter::TOUCH_BEGAN, ti[i].x / 100, ti[i].y/100);
                            }
                        }
                        else if(ti[i].dwFlags & TOUCHEVENTF_MOVE)
                        {
                            if (!osg_event) {
                                osg_event = getEventQueue()->touchMoved(  ti[i].dwID, osgGA::GUIEventAdapter::TOUCH_MOVED, ti[i].x/ 100, ti[i].y/ 100);
                            } else {
                                osg_event->addTouchPoint( ti[i].dwID, osgGA::GUIEventAdapter::TOUCH_MOVED, ti[i].x / 100, ti[i].y/100);
                            }
                        }
                        else if(ti[i].dwFlags & TOUCHEVENTF_UP)
                        {
                            // No double tap detection with RAW TOUCH Events, sorry.
                            if (!osg_event) {
                                osg_event = getEventQueue()->touchEnded( ti[i].dwID, osgGA::GUIEventAdapter::TOUCH_ENDED, ti[i].x/ 100, ti[i].y/ 100, 1);
                            } else {
                                osg_event->addTouchPoint( ti[i].dwID, osgGA::GUIEventAdapter::TOUCH_ENDED, ti[i].x / 100, ti[i].y/100);
                            }
                        }
                    }
                }
                if (closeTouchInputHandleFunc)
                    (*closeTouchInputHandleFunc)((HTOUCHINPUT)lParam);
                delete [] ti;
            }
            break;

        /////////////////
        default         :
        /////////////////

            if (_ownsWindow) return ::DefWindowProc(hwnd, uMsg, wParam, lParam);
            break;
    }

    if (_ownsWindow) return 0;

    return _windowProcedure==0 ? ::DefWindowProc(hwnd, uMsg, wParam, lParam) :
                                 ::CallWindowProc(_windowProcedure, hwnd, uMsg, wParam, lParam);
}


//////////////////////////////////////////////////////////////////////////////
//  Class responsible for registering the Win32 Windowing System interface
//////////////////////////////////////////////////////////////////////////////

struct RegisterWindowingSystemInterfaceProxy
{
    RegisterWindowingSystemInterfaceProxy()
    {
        osg::GraphicsContext::setWindowingSystemInterface(Win32WindowingSystem::getInterface());
    }

    ~RegisterWindowingSystemInterfaceProxy()
    {
        if (osg::Referenced::getDeleteHandler())
        {
            osg::Referenced::getDeleteHandler()->setNumFramesToRetainObjects(0);
            osg::Referenced::getDeleteHandler()->flushAll();
        }

        osg::GraphicsContext::setWindowingSystemInterface(0);
    }
};

static RegisterWindowingSystemInterfaceProxy createWindowingSystemInterfaceProxy;

} // namespace OsgViewer


// declare C entry point for static compilation.
extern "C" void OSGVIEWER_EXPORT graphicswindow_Win32(void)
{
    osg::GraphicsContext::setWindowingSystemInterface(osgViewer::Win32WindowingSystem::getInterface());
}


void GraphicsWindowWin32::raiseWindow()
{

    SetWindowPos(_hwnd, HWND_TOPMOST, _traits->x, _traits->y, _traits->width, _traits->height, SWP_NOMOVE|SWP_NOSIZE);
    SetWindowPos(_hwnd, HWND_NOTOPMOST, _traits->x, _traits->y, _traits->width, _traits->height, SWP_NOMOVE|SWP_NOSIZE);

}

