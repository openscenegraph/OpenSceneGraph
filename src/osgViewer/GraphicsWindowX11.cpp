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
*/

/* Note, elements of GraphicsWindowX11 have used Prodcer/RenderSurface_X11.cpp as both
 * a guide to use of X11/GLX and copiying directly in the case of setBorder().
 * These elements are license under OSGPL as above, with Copyright (C) 2001-2004  Don Burns.
 */

#include <osgViewer/GraphicsWindowX11>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <X11/Xmd.h>
#include <X11/keysym.h>
#include <X11/Xmu/WinUtil.h>
#include <X11/cursorfont.h>
#include <X11/Intrinsic.h>

#include <X11/Xmd.h>		/* For CARD16 */

using namespace osgViewer;

namespace osgViewer
{

class GraphicsContextX11 : public osg::GraphicsContext
{
    public:

        GraphicsContextX11(osg::GraphicsContext::Traits* traits):
            _valid(false),
            _display(0),
            _parent(0),
            _window(0)
        {
            _traits = traits;
        }
    
        virtual bool valid() const { return _valid; }

        /** Realise the GraphicsContext implementation, 
          * Pure virtual - must be implemented by concrate implementations of GraphicsContext. */
        virtual bool realizeImplementation() { osg::notify(osg::NOTICE)<<"GraphicsWindow::realizeImplementation() not implemented."<<std::endl; return false; }

        /** Return true if the graphics context has been realised, and is ready to use, implementation.
          * Pure virtual - must be implemented by concrate implementations of GraphicsContext. */
        virtual bool isRealizedImplementation() const  { osg::notify(osg::NOTICE)<<"GraphicsWindow::isRealizedImplementation() not implemented."<<std::endl; return false; }

        /** Close the graphics context implementation.
          * Pure virtual - must be implemented by concrate implementations of GraphicsContext. */
        virtual void closeImplementation()  { osg::notify(osg::NOTICE)<<"GraphicsWindow::closeImplementation() not implemented."<<std::endl; }

        /** Make this graphics context current implementation.
          * Pure virtual - must be implemented by concrate implementations of GraphicsContext. */
        virtual void makeCurrentImplementation()  { osg::notify(osg::NOTICE)<<"GraphicsWindow::makeCurrentImplementation() not implemented."<<std::endl; }
        
        /** Make this graphics context current with specified read context implementation.
          * Pure virtual - must be implemented by concrate implementations of GraphicsContext. */
        virtual void makeContextCurrentImplementation(GraphicsContext* /*readContext*/)  { osg::notify(osg::NOTICE)<<"GraphicsWindow::makeContextCurrentImplementation(..) not implemented."<<std::endl; }

        /** Pure virtual, Bind the graphics context to associated texture implementation.
          * Pure virtual - must be implemented by concrate implementations of GraphicsContext. */
        virtual void bindPBufferToTextureImplementation(GLenum /*buffer*/)  { osg::notify(osg::NOTICE)<<"GraphicsWindow::void bindPBufferToTextureImplementation(..) not implemented."<<std::endl; }

        /** Swap the front and back buffers implementation.
          * Pure virtual - must be implemented by Concrate implementations of GraphicsContext. */
        virtual void swapBuffersImplementation()  { osg::notify(osg::NOTICE)<<"GraphicsWindow:: swapBuffersImplementation() not implemented."<<std::endl; }
        
    protected:
        
        bool        _valid;
        Display*    _display;
        Window      _parent;
        Window      _window;

};

}

class X11KeyboardMap
{
    public:

        X11KeyboardMap()
        {
            _keymap[XK_Escape       ] = osgGA::GUIEventAdapter::KEY_Escape;
            _keymap[XK_F1           ] = osgGA::GUIEventAdapter::KEY_F1;
            _keymap[XK_F2           ] = osgGA::GUIEventAdapter::KEY_F2;
            _keymap[XK_F3           ] = osgGA::GUIEventAdapter::KEY_F3;
            _keymap[XK_F4           ] = osgGA::GUIEventAdapter::KEY_F4;
            _keymap[XK_F5           ] = osgGA::GUIEventAdapter::KEY_F5;
            _keymap[XK_F6           ] = osgGA::GUIEventAdapter::KEY_F6;
            _keymap[XK_F7           ] = osgGA::GUIEventAdapter::KEY_F7;
            _keymap[XK_F8           ] = osgGA::GUIEventAdapter::KEY_F8;
            _keymap[XK_F9           ] = osgGA::GUIEventAdapter::KEY_F9;
            _keymap[XK_F10          ] = osgGA::GUIEventAdapter::KEY_F10;
            _keymap[XK_F11          ] = osgGA::GUIEventAdapter::KEY_F11;
            _keymap[XK_F12          ] = osgGA::GUIEventAdapter::KEY_F12;
            _keymap[XK_quoteleft    ] = '"';
            _keymap[XK_1            ] = '1';
            _keymap[XK_2            ] = '2';
            _keymap[XK_3            ] = '3';
            _keymap[XK_4            ] = '4';
            _keymap[XK_5            ] = '5';
            _keymap[XK_6            ] = '6';
            _keymap[XK_7            ] = '7';
            _keymap[XK_8            ] = '8';
            _keymap[XK_9            ] = '9';
            _keymap[XK_0            ] = '0';
            _keymap[XK_minus        ] = '-';
            _keymap[XK_equal        ] = '=';
            _keymap[XK_BackSpace    ] = osgGA::GUIEventAdapter::KEY_BackSpace;
            _keymap[XK_Tab          ] = osgGA::GUIEventAdapter::KEY_Tab;
            _keymap[XK_a            ] = 'A';
            _keymap[XK_b            ] = 'B';
            _keymap[XK_c            ] = 'C';
            _keymap[XK_d            ] = 'D';
            _keymap[XK_e            ] = 'E';
            _keymap[XK_f            ] = 'F';
            _keymap[XK_g            ] = 'G';
            _keymap[XK_h            ] = 'H';
            _keymap[XK_i            ] = 'I';
            _keymap[XK_j            ] = 'J';
            _keymap[XK_k            ] = 'K';
            _keymap[XK_l            ] = 'L';
            _keymap[XK_m            ] = 'M';
            _keymap[XK_n            ] = 'N';
            _keymap[XK_o            ] = 'O';
            _keymap[XK_p            ] = 'P';
            _keymap[XK_q            ] = 'Q';
            _keymap[XK_r            ] = 'R';
            _keymap[XK_s            ] = 'S';
            _keymap[XK_t            ] = 'T';
            _keymap[XK_u            ] = 'U';
            _keymap[XK_v            ] = 'V';
            _keymap[XK_w            ] = 'W';
            _keymap[XK_x            ] = 'X';
            _keymap[XK_y            ] = 'Y';
            _keymap[XK_z            ] = 'Z';
            _keymap[XK_bracketleft  ] = '(';
            _keymap[XK_bracketright ] = ')';
            _keymap[XK_backslash    ] = '\\';
            _keymap[XK_Caps_Lock    ] = osgGA::GUIEventAdapter::KEY_Caps_Lock;
            _keymap[XK_semicolon    ] = ';';
            _keymap[XK_apostrophe   ] = '\'';
            _keymap[XK_Return       ] = osgGA::GUIEventAdapter::KEY_Return;
            _keymap[XK_Shift_L      ] = osgGA::GUIEventAdapter::KEY_Shift_L;
            _keymap[XK_comma        ] = ',';
            _keymap[XK_period       ] = '.';
            _keymap[XK_slash        ] = '/';
            _keymap[XK_Shift_R      ] = osgGA::GUIEventAdapter::KEY_Shift_R;
            _keymap[XK_Control_L    ] = osgGA::GUIEventAdapter::KEY_Control_L;
            _keymap[XK_Super_L      ] = osgGA::GUIEventAdapter::KEY_Super_L;
            _keymap[XK_space        ] = ' ';
            _keymap[XK_Alt_L        ] = osgGA::GUIEventAdapter::KEY_Alt_L;
            _keymap[XK_Alt_R        ] = osgGA::GUIEventAdapter::KEY_Alt_R;
            _keymap[XK_Super_R      ] = osgGA::GUIEventAdapter::KEY_Super_R;
            _keymap[XK_Menu         ] = osgGA::GUIEventAdapter::KEY_Menu;
            _keymap[XK_Control_R    ] = osgGA::GUIEventAdapter::KEY_Control_R;
            _keymap[XK_Print        ] = osgGA::GUIEventAdapter::KEY_Print;
            _keymap[XK_Scroll_Lock  ] = osgGA::GUIEventAdapter::KEY_Scroll_Lock;
            _keymap[XK_Pause        ] = osgGA::GUIEventAdapter::KEY_Pause;
            _keymap[XK_Home         ] = osgGA::GUIEventAdapter::KEY_Home;
            _keymap[XK_Page_Up      ] = osgGA::GUIEventAdapter::KEY_Page_Up;
            _keymap[XK_End          ] = osgGA::GUIEventAdapter::KEY_End;
            _keymap[XK_Page_Down    ] = osgGA::GUIEventAdapter::KEY_Page_Down;
            _keymap[XK_Delete       ] = osgGA::GUIEventAdapter::KEY_Delete;
            _keymap[XK_Insert       ] = osgGA::GUIEventAdapter::KEY_Insert;
            _keymap[XK_Left         ] = osgGA::GUIEventAdapter::KEY_Left;
            _keymap[XK_Up           ] = osgGA::GUIEventAdapter::KEY_Up;
            _keymap[XK_Right        ] = osgGA::GUIEventAdapter::KEY_Right;
            _keymap[XK_Down         ] = osgGA::GUIEventAdapter::KEY_Down;
            _keymap[XK_Num_Lock     ] = osgGA::GUIEventAdapter::KEY_Num_Lock;
            _keymap[XK_KP_Divide    ] = osgGA::GUIEventAdapter::KEY_KP_Divide;
            _keymap[XK_KP_Multiply  ] = osgGA::GUIEventAdapter::KEY_KP_Multiply;
            _keymap[XK_KP_Subtract  ] = osgGA::GUIEventAdapter::KEY_KP_Subtract;
            _keymap[XK_KP_Add       ] = osgGA::GUIEventAdapter::KEY_KP_Add;
            _keymap[XK_KP_Home      ] = osgGA::GUIEventAdapter::KEY_KP_Home;
            _keymap[XK_KP_Up        ] = osgGA::GUIEventAdapter::KEY_KP_Up;
            _keymap[XK_KP_Page_Up   ] = osgGA::GUIEventAdapter::KEY_KP_Page_Up;
            _keymap[XK_KP_Left      ] = osgGA::GUIEventAdapter::KEY_KP_Left;
            _keymap[XK_KP_Begin     ] = osgGA::GUIEventAdapter::KEY_KP_Begin;
            _keymap[XK_KP_Right     ] = osgGA::GUIEventAdapter::KEY_KP_Right;
            _keymap[XK_KP_End       ] = osgGA::GUIEventAdapter::KEY_KP_End;
            _keymap[XK_KP_Down      ] = osgGA::GUIEventAdapter::KEY_KP_Down;
            _keymap[XK_KP_Page_Down ] = osgGA::GUIEventAdapter::KEY_KP_Page_Down;
            _keymap[XK_KP_Insert    ] = osgGA::GUIEventAdapter::KEY_KP_Insert;
            _keymap[XK_KP_Delete    ] = osgGA::GUIEventAdapter::KEY_KP_Delete;
            _keymap[XK_KP_Enter     ] = osgGA::GUIEventAdapter::KEY_KP_Enter;
     
        }

        ~X11KeyboardMap() {}

        int remapKey(int key)
        {
            KeyMap::iterator itr = _keymap.find(key);
            if (itr == _keymap.end()) return key;
            else return itr->second;
        }
      
    protected:
    

        typedef std::map<int, int> KeyMap;
        KeyMap _keymap;
};

static int remapX11Key(int key)
{
    static X11KeyboardMap s_x11KeyboardMap;
    return s_x11KeyboardMap.remapKey(key);
}

GraphicsWindowX11::~GraphicsWindowX11()
{
    close(true);
}

bool GraphicsWindowX11::createVisualInfo()
{
    typedef std::vector<int> Attributes;
    Attributes attributes;
    
    attributes.push_back(GLX_USE_GL);
    
    attributes.push_back(GLX_RGBA);
    
    if (_traits->doubleBuffer) attributes.push_back(GLX_DOUBLEBUFFER);
    
    if (_traits->quadBufferStereo) attributes.push_back(GLX_STEREO);
    
    attributes.push_back(GLX_RED_SIZE); attributes.push_back(_traits->red);
    attributes.push_back(GLX_GREEN_SIZE); attributes.push_back(_traits->green);
    attributes.push_back(GLX_BLUE_SIZE); attributes.push_back(_traits->blue);
    attributes.push_back(GLX_DEPTH_SIZE); attributes.push_back(_traits->depth);
    
    if (_traits->alpha) { attributes.push_back(GLX_ALPHA_SIZE); attributes.push_back(_traits->alpha); }
    
    if (_traits->stencil) { attributes.push_back(GLX_STENCIL_SIZE); attributes.push_back(_traits->stencil); }

#if defined(GLX_SAMPLE_BUFFERS) && defined (GLX_SAMPLES)

    if (_traits->sampleBuffers) { attributes.push_back(GLX_SAMPLE_BUFFERS); attributes.push_back(_traits->sampleBuffers); }
    if (_traits->sampleBuffers) { attributes.push_back(GLX_SAMPLES); attributes.push_back(_traits->samples); }

#endif
    // TODO
    //  GLX_AUX_BUFFERS
    //  GLX_ACCUM_RED_SIZE
    //  GLX_ACCUM_GREEN_SIZE
    //  GLX_SAMPLE_BUFFERS
    //  GLX_SAMPLES
    
    attributes.push_back(None);
    
    _visualInfo = glXChooseVisual( _display, _traits->screenNum, &(attributes.front()) );

    return _visualInfo != 0;
}

void GraphicsWindowX11::setWindowDecoration(bool flag)
{
    Atom atom;
    if( (atom = XInternAtom( _display, "_MOTIF_WM_HINTS", 0 )) != None )
    {
// Hack for sending 64 bit atom to Xserver
#if defined( _MIPS_SIM) && (_MIPS_SIM == _MIPS_SIM_ABI64) || defined ( __ia64 ) || defined (__amd64 ) || defined(__x86_64__)
        struct {
            CARD32 flags0;
            CARD32 flags1;
            CARD32 functions0;
            CARD32 functions1;
            CARD32 decorations0;
            CARD32 decorations1;
            INT32 input_mode0;
            INT32 input_mode1;
            CARD32 status0;
            CARD32 status1;
        } wmHints;

#if defined( __ia64 ) || defined (__amd64)  || defined (__x86_64__)
        wmHints.flags0       = (1L << 1);
        wmHints.functions0   = 0;
        wmHints.decorations0 = flag;
        wmHints.input_mode0  = 0;

#else
        wmHints.flags1       = (1L << 1);
        wmHints.functions1   = 0;
        wmHints.decorations1 = flag;
        wmHints.input_mode1  = 0;
#endif

#else

        struct {
            CARD32 flags;
            CARD32 functions;
            CARD32 decorations;
            INT32 input_mode;
            CARD32 status;
        } wmHints;

        wmHints.flags       = (1L << 1);
        wmHints.functions   = 0;
        wmHints.decorations = flag;
        wmHints.input_mode  = 0;
#endif

        XUnmapWindow(_display, _window );
        XChangeProperty( _display, _window, atom, atom, 32, PropModeReplace, (unsigned char *)&wmHints,  5 );
        XMapWindow(_display, _window );

        XFlush(_display);
        XSync(_display,0);

#if 0
        // now update the window dimensions to account for any size changes made by the window manager,
        XGetWindowAttributes( _display, _window, &watt );
        _traits->width = watt.width;
        _traits->height = watt.height;
#endif

    }
    else
        osg::notify(osg::NOTICE)<<"Error: GraphicsWindowX11::setBorder(" << flag << ") - couldn't change decorations." << std::endl;
}

void GraphicsWindowX11::useCursor(bool cursorOn)
{
    if (cursorOn)
    {
        _currentCursor = _defaultCursor;
    }
    else
    {
        _currentCursor = _nullCursor;
    }

    if (_display && _window)
    {
        XDefineCursor( _display, _window, _currentCursor );
        XFlush(_display);
        XSync(_display,0);
    }
}

void GraphicsWindowX11::init()
{
    if (_initialized) return;

    if (!_traits)
    {
        _valid = false;
        return;
    }

    _display = XOpenDisplay(_traits->displayName().c_str());
    
    unsigned int screen = _traits->screenNum;

    if (!_display)
    {
        osg::notify(osg::NOTICE)<<"Error: Unable to open display \"" << XDisplayName(_traits->displayName().c_str()) << "\"."<<std::endl;
        _valid = false;
        return;
    }

     // Query for GLX extension
    int errorBase, eventBase;
    if( glXQueryExtension( _display, &errorBase, &eventBase)  == False )
    {
        osg::notify(osg::NOTICE)<<"Error: " << XDisplayName(_traits->displayName().c_str()) <<" has no GLX extension." << std::endl;

        XCloseDisplay( _display );
        _display = 0;
        _valid = false;
        return;
    }
    
    // osg::notify(osg::NOTICE)<<"GLX extension, errorBase="<<errorBase<<" eventBase="<<eventBase<<std::endl;

    if (!createVisualInfo())
    {
        osg::notify(osg::NOTICE)<<"Error: Not able to create requested visual." << std::endl;
        XCloseDisplay( _display );
        _display = 0;
        _valid = false;
        return;
    }
    
    // need to pick up from traits
    GLXContext sharedGLContext = 0;
    
    _glxContext = glXCreateContext( _display, _visualInfo, sharedGLContext, True );
    
    if (!_glxContext)
    {
        osg::notify(osg::NOTICE)<<"Error: Unable to create OpenGL graphics context."<<std::endl;
        XCloseDisplay( _display );
        _display = 0;
        _valid = false;
        return;
    }
    
    _parent = RootWindow( _display, screen );

    XWindowAttributes watt;
    XGetWindowAttributes( _display, _parent, &watt );
    // unsigned int parentWindowHeight = watt.height;

    XSetWindowAttributes swatt;
    swatt.colormap = XCreateColormap( _display, _parent, _visualInfo->visual, AllocNone);
    //swatt.colormap = DefaultColormap( _dpy, 10 );
    swatt.background_pixel = 0;
    swatt.border_pixel = 0;
    swatt.event_mask =  0;
    unsigned long mask = CWBackPixel | CWBorderPixel | CWEventMask | CWColormap;

    bool overrideRedirect = false;
    if (overrideRedirect)
    {
        swatt.override_redirect = true;
        mask |= CWOverrideRedirect;
    }

    _window = XCreateWindow( _display, _parent,
                             _traits->x,
                             _traits->y,
                             _traits->width, _traits->height, 0,
                             _visualInfo->depth, InputOutput,
                             _visualInfo->visual, mask, &swatt );
                             
    if (!_window)
    {
        osg::notify(osg::NOTICE)<<"Error: Unable to create Window."<<std::endl;
        XCloseDisplay( _display );
        _display = 0;
        _glxContext = 0;
        _valid = false;
        return;
    }

    // This positions the window at _windowX, _windowY
    XSizeHints sh;
    sh.flags = 0;
    sh.flags |= USSize;
    sh.flags &= 0x7;
    sh.flags |= USPosition;
    sh.flags &= 0xB;
    sh.x = _traits->x;
    sh.y = _traits->y;
    sh.width  = _traits->width;
    sh.height = _traits->height;
    XSetStandardProperties( _display, _window, _traits->windowName.c_str(), _traits->windowName.c_str(), None, 0, 0, &sh);

#if 1
    setWindowDecoration(_traits->windowDecoration);
#else
    setWindowDecoration(true);
#endif
    // Create  default Cursor
    _defaultCursor = XCreateFontCursor( _display, XC_left_ptr );


    // Create Null Cursor
    {
        Pixmap pixmap;
        static char buff[2] = {0,0};
        static XColor ncol = {0,0,0,0,DoRed|DoGreen|DoBlue,0};

        pixmap = XCreateBitmapFromData( _display, _parent, buff, 1, 1);
        _nullCursor = XCreatePixmapCursor( _display, pixmap, pixmap, &ncol, &ncol, 0, 0 );
    }

    useCursor(true);

    XSelectInput( _display, _window, ExposureMask | StructureNotifyMask | 
                                     KeyPressMask | KeyReleaseMask |
                                     PointerMotionMask  | ButtonPressMask | ButtonReleaseMask);

    XFlush( _display );
    XSync( _display, 0 );

    // now update the window dimensions to account for any size changes made by the window manager,
    XGetWindowAttributes( _display, _window, &watt );
    
    if (_traits->width != watt.width && _traits->height != watt.height)
    {
        resized( _traits->x, _traits->y, _traits->width, _traits->height );
    }
        
    //osg::notify(osg::NOTICE)<<"After sync apply.x = "<<watt.x<<" watt.y="<<watt.y<<" width="<<watt.width<<" height="<<watt.height<<std::endl;

    _deleteWindow = XInternAtom (_display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(_display, _window, &_deleteWindow, True); 

    _valid = true;
    _initialized = true;
}

bool GraphicsWindowX11::realizeImplementation()
{
    if (_realized)
    {
        osg::notify(osg::NOTICE)<<"GraphicsWindowX11::realizeImplementation() Already realized"<<std::endl;
        return true;
    }

    if (!_initialized) init();
    
    if (!_initialized) return false;
    
    XMapWindow( _display, _window );
    
//    Window temp = _window;
//    XSetWMColormapWindows( _display, _window, &temp, 1);
    
    _realized = true;

    return true;
}

void GraphicsWindowX11::makeCurrentImplementation()
{
    if (!_realized)
    {
        osg::notify(osg::NOTICE)<<"Warning: GraphicsWindow not realized, cannot do makeCurrent."<<std::endl;
        return;
    }

    // osg::notify(osg::NOTICE)<<"makeCurrentImplementation "<<this<<" "<<OpenThreads::Thread::CurrentThread()<<std::endl;
    // osg::notify(osg::NOTICE)<<"   glXMakeCurrent ("<<_display<<","<<_window<<","<<_glxContext<<std::endl;

    // checkEvents();

    // XFlush(_display);
    // XSync(_display,0);

    if (glXMakeCurrent( _display, _window, _glxContext ))
    {
        // osg::notify(osg::NOTICE)<<"makeCurrentImplementation sucessful"<<std::endl;
    }
    else
    {

        osg::notify(osg::NOTICE)<<"makeCurrentImplementation failed"<<std::endl;
        
        while(!glXMakeCurrent( _display, _window, _glxContext )) 
        {
            OpenThreads::Thread::YieldCurrentThread();
            osg::notify(osg::NOTICE)<<"  makeCurrentImplementation failed again."<<std::endl;
        }
        
        // osg::notify(osg::NOTICE)<<"  Succeeded at last."<<std::endl;

    }
    
    

}


void GraphicsWindowX11::closeImplementation()
{
    // osg::notify(osg::NOTICE)<<"Closing GraphicsWindowX11"<<std::endl;

    if (_display && _window)
    {
        glXDestroyContext(_display, _glxContext );
        
        XDestroyWindow(_display, _window);

        XFlush( _display );
        XSync( _display,0 );

        XCloseDisplay( _display );
    }

    _window = 0;
    _parent = 0;
    _glxContext = 0;

    if(_visualInfo)
    {
        XFree(_visualInfo);
        _visualInfo = 0;
    }

    _initialized = false;
    _realized = false;
    _valid = false;
}

void GraphicsWindowX11::swapBuffersImplementation()
{
    if (!_realized) return;

    // osg::notify(osg::NOTICE)<<"swapBuffersImplementation "<<this<<" "<<OpenThreads::Thread::CurrentThread()<<std::endl;

    glXSwapBuffers(_display, _window); 
    
#if 0   
    checkEvents();
#endif    

    // glFlush();
}

void GraphicsWindowX11::checkEvents()
{
    if (!_realized) return;

    int windowX = _traits->x;
    int windowY = _traits->y;
    int windowWidth = _traits->width;
    int windowHeight = _traits->height;

    bool destroyWindowRequested = false;
     
         // osg::notify(osg::NOTICE)<<"Check events"<<std::endl;    
    while( XPending(_display) )
    {
        XEvent ev;
        XNextEvent( _display, &ev );

        switch( ev.type )
        {
            case ClientMessage:
            {
                if (static_cast<Atom>(ev.xclient.data.l[0]) == _deleteWindow)
                {
                    osg::notify(osg::INFO)<<"DeleteWindow event recieved"<<std::endl;
                    destroyWindowRequested = true;
                }
            }
            case Expose :
                osg::notify(osg::INFO)<<"Expose x="<<ev.xexpose.x<<" y="<<ev.xexpose.y<<" width="<<ev.xexpose.width<<", height="<<ev.xexpose.height<<std::endl;
                break;

            case GravityNotify :
                osg::notify(osg::INFO)<<"GravityNotify event recieved"<<std::endl;
                break;

            case UnmapNotify :
                osg::notify(osg::INFO)<<"UnmapNotify event recieved"<<std::endl;
                break;

            case ReparentNotify:
                osg::notify(osg::INFO)<<"ReparentNotify event recieved"<<std::endl;
                break;

            case DestroyNotify :
                osg::notify(osg::INFO)<<"DestroyNotify event recieved"<<std::endl;
                _realized =  false;
                _valid = false;
                break;
                
            case ConfigureNotify :
            {
                osg::notify(osg::INFO)<<"ConfigureNotify x="<<ev.xconfigure.x<<" y="<<ev.xconfigure.y<<" width="<<ev.xconfigure.width<<", height="<<ev.xconfigure.height<<std::endl;

                if (windowX != ev.xconfigure.x || 
                    windowX != ev.xconfigure.y ||
                    windowWidth != ev.xconfigure.width ||
                    windowHeight != ev.xconfigure.height)
                {
                    windowX = ev.xconfigure.x;
                    windowY = ev.xconfigure.y;
                    windowWidth = ev.xconfigure.width;
                    windowHeight = ev.xconfigure.height;
                }

                break;
            }
            
            case MapNotify :
            {
                osg::notify(osg::INFO)<<"MapNotify"<<std::endl;
                XWindowAttributes watt;
                do
                XGetWindowAttributes(_display, _window, &watt );
                while( watt.map_state != IsViewable );
                
                osg::notify(osg::INFO)<<"MapNotify x="<<watt.x<<" y="<<watt.y<<" width="<<watt.width<<", height="<<watt.height<<std::endl;

                if (windowWidth != watt.width || windowHeight != watt.height)
                {
                    windowWidth = watt.width;
                    windowHeight = watt.height;
                }
                
                _traits->width = watt.width;
                _traits->height = watt.height;
                

                break;
            }

           case MotionNotify :
           {
                osg::notify(osg::INFO)<<"MotionNotify time="<<ev.xmotion.time<<std::endl;
           
                int  wx, wy;
                Window win = 0L;
                if( ev.xmotion.same_screen )
                {
                    wx = ev.xmotion.x;
                    wy = ev.xmotion.y;
                }
                else
                {
                    // the mouse in on another screen so need to compute the
                    // coordinates of the mouse position relative to an absolute position
                    // then take away the position of the original window/screen to get
                    // the coordinates relative to the original position.
                    Window root;
                    int rx, ry;
                    unsigned int buttons;

                    int screenOrigin_x = 0;
                    int screenOrigin_y = 0;
                    int i;
                    for(i= 0; i < ScreenCount(_display); i++ )
                    {
                        if( XQueryPointer( _display, RootWindow(_display, i),
                              &root, &win, &rx, &ry, &wx, &wy, &buttons) )
                        {
                            break;
                        }

                        screenOrigin_x += DisplayWidth(_display, i);
                    }

                    for(i= 0; i < static_cast<int>(_traits->screenNum); i++ )
                    {
                        screenOrigin_x -= DisplayWidth(_display, i);
                    }

                    int dest_x_return, dest_y_return;
                    Window child_return;
                    XTranslateCoordinates(_display, _window, _parent, 0, 0, &dest_x_return, &dest_y_return, &child_return);

                    wx += (screenOrigin_x - dest_x_return);
                    wy += (screenOrigin_y - dest_y_return);
                }
                

                float mx = wx;
                float my = wy;
                transformMouseXY(mx, my);
                getEventQueue()->mouseMotion(mx, my);

                // osg::notify(osg::NOTICE)<<"MotionNotify wx="<<wx<<" wy="<<wy<<" mx="<<mx<<" my="<<my<<std::endl;

                break;
            }
            
            case ButtonPress :
            {
                if( ev.xbutton.button == Button4 )
                {
                    getEventQueue()->mouseScroll(osgGA::GUIEventAdapter::SCROLL_UP);
                }
                else if( ev.xbutton.button == Button5)
                {
                    getEventQueue()->mouseScroll(osgGA::GUIEventAdapter::SCROLL_DOWN);
                }
                else
                {
                    float mx = ev.xbutton.x;
                    float my = ev.xmotion.y;
                    transformMouseXY(mx, my);
                    getEventQueue()->mouseButtonPress(mx, my, ev.xbutton.button);
                }
                break;
            }
            
            case ButtonRelease :
            {
                if( ev.xbutton.button == Button4 )
                {
                    getEventQueue()->mouseScroll(osgGA::GUIEventAdapter::SCROLL_UP);
                }
                else if( ev.xbutton.button == Button5)
                {
                    getEventQueue()->mouseScroll(osgGA::GUIEventAdapter::SCROLL_DOWN);
                }
                else
                {
                    float mx = ev.xbutton.x;
                    float my = ev.xmotion.y;
                    transformMouseXY(mx, my);
                    getEventQueue()->mouseButtonRelease(mx, my, ev.xbutton.button);
                }
                break;
            }
            
            case KeyPress:
            {
                int keySymbol = 0;
                unsigned int modifierMask = 0;
                adaptKey(ev.xkey, keySymbol, modifierMask);

                getEventQueue()->keyPress(keySymbol);
                getEventQueue()->getCurrentEventState()->setModKeyMask(modifierMask);
                break;
            }
            
            case KeyRelease:
            {
                int keySymbol = 0;
                unsigned int modifierMask = 0;
                adaptKey(ev.xkey, keySymbol, modifierMask);
                
                getEventQueue()->keyRelease(keySymbol);
                getEventQueue()->getCurrentEventState()->setModKeyMask(modifierMask);
                break;
            }
            
            default:
                osg::notify(osg::NOTICE)<<"Other event"<<std::endl;
                break;
                
        }
    }

    if (windowX != _traits->x || 
        windowY != _traits->y ||
        windowWidth != _traits->width ||
        windowHeight != _traits->height)
    {
        resized(windowX, windowY, windowWidth, windowHeight); 
        getEventQueue()->windowResize(windowX, windowY, windowWidth, windowHeight);
    }

    if (destroyWindowRequested)
    {
        close();
    }

}

void GraphicsWindowX11::grabFocus()
{
    XSetInputFocus( _display, _window, RevertToNone, CurrentTime );
    XFlush(_display); XSync(_display,0);
}

void GraphicsWindowX11::grabFocusIfPointerInWindow()
{
    Window win, root;
    int wx, wy, rx, ry;
    unsigned int buttons;

    if( XQueryPointer( _display, _window,
          &root, &win, &rx, &ry, &wx, &wy, &buttons))
    {
        grabFocus();
    }
}


void GraphicsWindowX11::transformMouseXY(float& x, float& y)
{
    if (getEventQueue()->getUseFixedMouseInputRange())
    {
        osgGA::GUIEventAdapter* eventState = getEventQueue()->getCurrentEventState();
        x = eventState->getXmin() + (eventState->getXmax()-eventState->getXmin())*x/float(_traits->width);
        y = eventState->getYmin() + (eventState->getYmax()-eventState->getYmin())*y/float(_traits->height);
    }
}

void GraphicsWindowX11::adaptKey(XKeyEvent& keyevent, int& keySymbol, unsigned int& modifierMask)
{
 
    static XComposeStatus state;
    unsigned char keybuf[32];
    XLookupString( &keyevent, (char *)keybuf, sizeof(keybuf), NULL, &state );

    modifierMask = 0;
    if( keyevent.state & ShiftMask )
    {
        modifierMask |= osgGA::GUIEventAdapter::MODKEY_SHIFT;
    }
    if( keyevent.state & LockMask )
    {
        modifierMask |= osgGA::GUIEventAdapter::MODKEY_CAPS_LOCK;
    }
    if( keyevent.state & ControlMask )
    {
        modifierMask |= osgGA::GUIEventAdapter::MODKEY_CTRL;
    }
    if( keyevent.state & Mod1Mask )
    {
        modifierMask |= osgGA::GUIEventAdapter::MODKEY_ALT;
    }
    if( keyevent.state & Mod2Mask )
    {
        modifierMask |= osgGA::GUIEventAdapter::MODKEY_NUM_LOCK;
    }
    if( keyevent.state & Mod4Mask )
    {
        modifierMask |= osgGA::GUIEventAdapter::MODKEY_META;
    }

    keySymbol = keybuf[0];
    
    KeySym ks = XKeycodeToKeysym( _display, keyevent.keycode, 0 );
    int remappedKey = remapX11Key(ks);
    if (remappedKey & 0xff00) 
    {
        // special keyboard character
        keySymbol = remappedKey;
    }
    else
    {
        // normal ascii key
        keySymbol = keybuf[0];
    }
    
    
}

void GraphicsWindowX11::requestWarpPointer(float x,float y)
{
    XWarpPointer( _display, 
                  None,
                  _window, 
                  0, 0, 0, 0,
                  static_cast<int>(x), static_cast<int>(y) );

    XFlush(_display);
    XSync(_display, 0);
    
    getEventQueue()->mouseWarped(x,y);
}

int X11ErrorHandling(Display* display, XErrorEvent* event)
{
    osg::notify(osg::NOTICE)<<"Got an X11ErrorHandling call display="<<display<<" event="<<event<<std::endl;
    return 0;
}


struct X11WindowingSystemInterface : public osg::GraphicsContext::WindowingSystemInterface
{

    X11WindowingSystemInterface()
    {
        XSetErrorHandler(X11ErrorHandling);
    
#if 1    
        if (XInitThreads() == 0)
        {
            osg::notify(osg::NOTICE) << "Error: XInitThreads() failed. Aborting." << std::endl;
            exit(1);
        }
        else
        {
            osg::notify(osg::INFO) << "X11WindowingSystemInterface, xInitThreads() multi-threaded X support initialized.\n";
        }
#endif        
    }

    ~X11WindowingSystemInterface()
    {
        XSetErrorHandler(0);
    }

    virtual unsigned int getNumScreens(const osg::GraphicsContext::ScreenIdentifier& si) 
    {
        Display* display = XOpenDisplay(si.displayName().c_str());
        if(display)
        {
            unsigned int numScreens = ScreenCount(display); 
            XCloseDisplay(display);

            return numScreens;
        }
        else
        {
            osg::notify(osg::NOTICE) << "A Unable to open display \"" << XDisplayName(si.displayName().c_str()) << "\""<<std::endl;
            return 0;
        }
    }

    virtual void getScreenResolution(const osg::GraphicsContext::ScreenIdentifier& si, unsigned int& width, unsigned int& height)
    {
        Display* display = XOpenDisplay(si.displayName().c_str());
        if(display)
        {
            width = DisplayWidth(display, si.screenNum);
            height = DisplayHeight(display, si.screenNum);
            XCloseDisplay(display);
        }
        else
        {
            osg::notify(osg::NOTICE) << "Unable to open display \"" << XDisplayName(si.displayName().c_str()) << "\"."<<std::endl;
            width = 0;
            height = 0;
        }
    }

    virtual osg::GraphicsContext* createGraphicsContext(osg::GraphicsContext::Traits* traits)
    {
        if (traits->pbuffer)
        {
            osg::ref_ptr<osgViewer::GraphicsContextX11> pbuffer = new GraphicsContextX11(traits);
            if (pbuffer->valid()) return pbuffer.release();
            else return 0;
        }
        else
        {
            osg::ref_ptr<osgViewer::GraphicsWindowX11> window = new GraphicsWindowX11(traits);
            if (window->valid()) return window.release();
            else return 0;
        }
    }

};

struct RegisterWindowingSystemInterfaceProxy
{
    RegisterWindowingSystemInterfaceProxy()
    {
        osg::GraphicsContext::setWindowingSystemInterface(new X11WindowingSystemInterface);
    }

    ~RegisterWindowingSystemInterfaceProxy()
    {
        osg::GraphicsContext::setWindowingSystemInterface(0);
    }
};

RegisterWindowingSystemInterfaceProxy createWindowingSystemInterfaceProxy;
