/* -*-c++-*- Producer - Copyright (C) 2001-2004  Don Burns
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


#ifndef OSGPRODUCER_RENDER_SURFACE
#define OSGPRODUCER_RENDER_SURFACE 1

#include <map>
#include <string>
#include <iostream>

#include <osg/Referenced>
#include <osg/ref_ptr>
#include "VisualChooser.h"

#ifdef _WIN32_IMPLEMENTATION
  #include <vector>
#endif

namespace osgProducer {

/**
    \class RenderSurface
    \brief A RenderSurface provides a rendering surface for 3D graphics applications.

    A RenderSurface creates a window in a windowing system for the purpose of 3D
    rendering.  The focus of a RenderSurface differs from a windowing system window
    in that it is not a user input/output device, but rather a context and screen area
    specifically designed for 3D applications.  Consequently, a RenderSurface does not
    provide or impose a requirement on the caller to structure the application around
    the capturing or handling of events.  Further, RenderSurface provides increased
    control over the quality of pixel formats.
 */

class RenderSurface : public osg::Referenced
{
    public :

        static const unsigned int UnknownDimension;
        static const unsigned int UnknownAmount;
        static unsigned int _numScreens;

        class Callback : public osg::Referenced
        {
            public:
                Callback() {}
                virtual void operator()( const RenderSurface & ) = 0;

            protected:
                virtual ~Callback() {}
        };

        struct InputRectangle
        {
            public:
                InputRectangle(): _left(-1.0), _bottom(-1.0), _width(2.0), _height(2.0) {}
                InputRectangle( float left, float right, float bottom, float top ):
                    _left(left), _bottom(bottom), _width(right-left), _height(top-bottom) {}
                InputRectangle(const InputRectangle &ir)
                {
                     _left = ir._left;
                     _bottom = ir._bottom;
                     _width = ir._width;
                     _height = ir._height;
                }
                virtual ~InputRectangle() {}

                void set( float left, float right, float bottom, float top )
                {
                    _left = left;
                    _bottom = bottom;
                    _width = right - left;
                    _height = top - bottom;
                }
                float left() const { return _left; }
                float bottom() const { return _bottom; }
                float width() const { return _width; }
                float height() const { return _height; }

            protected:

            private:
                float _left, _bottom, _width, _height;
        };

        enum DrawableType {
            DrawableType_Window,
            DrawableType_PBuffer
        };


        RenderSurface( void );

        static unsigned int getNumberOfScreens(void);

        void setDrawableType( DrawableType );
        DrawableType getDrawableType();
        void setReadDrawable( RenderSurface *);
        RenderSurface* getReadDrawable() { return _readDrawableRenderSurface; }

        void setInputRectangle( const InputRectangle &ir );
        const InputRectangle &getInputRectangle() const;
        void bindInputRectangleToWindowSize(bool);


        /** Set the name of the Host the window is to be created on.
            Ignored on Win32*/
        void setHostName( const std::string & );

        /**
          * Get the name of the Host the window is to be created on.
          * Ignored on Win32 */
        const std::string& getHostName( void ) const;

        /**
          * Set the number of the display the render surface is to
          * be created on.  In XWindows, this is the number of the
          * XServer.  Ignored on Win32   */
        void setDisplayNum( int );

        /** Get the number of the display the render surface is to
          * be created on.  In XWindows, this is the number of the
          * XServer.  Ignored on Win32   */
        int getDisplayNum( void ) const;

        /** Set the number of the screen the render surface is to
          * be created on.  In XWindows, this is the number of the
          * XServer.  Ignored on Win32   */
        void setScreenNum( int );

        /** Get the number of the screen the render surface is to
          * be created on.  In XWindows, this is the number of the
          * XServer.  Ignored on Win32   */
        int getScreenNum( void ) const;

        /** Get the size of the screen in pixels the render surface
          * is to be created on.  */
        void getScreenSize( unsigned int &width, unsigned int &height ) const;


        /** Default window name */
        static const std::string defaultWindowName;

        static const std::string &getDefaultWindowName();

        /** Set the Window system Window name of the Render Surface */
        void setWindowName( const std::string & );
        /** Get the Window system Window name of the Render Surface */
        const std::string& getWindowName( void ) const;

        /** Set the windowing system rectangle the RenderSurface will
          * occupy on the screen.  The parameters are given as integers
          * in screen space.  x and y determine the lower left hand corner
          * of the RenderSurface.  Width and height are given in screen
          * coordinates */
        void  setWindowRectangle( int x, int y, unsigned int width, unsigned int height,
                                    bool resize=true );
        /** Get the windowing system rectangle the RenderSurface will
          * occupy on the screen.  The parameters are given as integers
          * in screen space.  x and y determine the lower left hand corner
          * of the RenderSurface.  Width and height are given in screen
          * coordinates */
        void getWindowRectangle( int &x, int &y, unsigned int &width, unsigned int &height ) const;
        /** Get the X coordinate of the origin of the RenderSurface's window */
        int getWindowOriginX() const;

        /** Get the Y coordinate of the origin of the RenderSurface's window */
        int getWindowOriginY() const;

        /** Get the width of the RenderSurface in windowing system screen coordinates */
        unsigned int getWindowWidth() const;

        /** Get the height of the RenderSurface in windowing system screen coordinates */
        unsigned int getWindowHeight() const;

#if 0
        /** Explicitly set the Display variable before realization. (X11 only). */
        void               setDisplay( Display *dpy );
        /** Get the Display. (X11 only). */
        Display*           getDisplay( void );
        /** Get the const Display. (X11 only). */
        const Display*     getDisplay( void ) const;

        /** Explicitly set the Windowing system window before realization. */
        void               setWindow( const Window win );
        /** Returns the Windowing system handle to the window */
        Window getWindow( void ) const;

        void setGLContext( GLContext );
        /** Returns the OpenGL context */
        GLContext getGLContext( void ) const;

        /** Set the Windowing system's parent window */
        void setParentWindow( Window parent );
        /** Get the Windowing system's parent window */
        Window getParentWindow( void ) const;

#endif

        void setVisualChooser( VisualChooser *vc );
        VisualChooser *getVisualChooser( void );
        const VisualChooser *getVisualChooser( void ) const;

#if 0
        void setVisualInfo( VisualInfo *vi );
        VisualInfo *getVisualInfo( void );
        const VisualInfo *getVisualInfo( void ) const;

        /** Returns true if the RenderSurface has been realized, false if not. */
        bool isRealized( void ) const;
#endif

        /** Request the use of a window border.  If flag is false, no border will
          * appear after realization.  If flag is true, the windowing system window
          * will be created in default state. */
        void useBorder( bool flag );
        bool usesBorder();

        /** Use overrideRedirect (X11 only).  This bypasses the window manager
          * control over the Window.  Ignored on Win32 and Mac CGL versions.
          * This call will only have effect if called before realize().  Calling
          * it subsequent to realize() will issue a warning. */
         void useOverrideRedirect(bool);
         bool usesOverrideRedirect();

       /** Request whether the window should have a visible cursor.  If true, the
         * windowing system's default cursor will be assigned to the window.  If false
         * the window will not have a visible cursor. */
        void useCursor( bool flag );

#if 0
        /** Set the current window cursor.  Producer provides no functionality to create
          * cursors.  It is the application's responsibility to create a Cursor using the
          * windowing system of choice.  setCursor() will simply set a predefined cursor
          * as the current Cursor */
        void setCursor( Cursor );
#endif
        void setCursorToDefault();

        /** Specify whether the RenderSurface should use a separate thread for
          * window configuration events.  If flag is set to true, then the
          * RenderSurface will spawn a new thread to manage events caused by
          * resizing the window, mapping or destroying the window. */
        void useConfigEventThread(bool flag);

        /** shareAllGLContexts will share all OpenGL Contexts between render surfaces
          * upon realize.  This must be called before any call to renderSurface::realize().
          */
        static void shareAllGLContexts(bool);

        /** Returns true or false indicating the state flag for sharing contexts
          * between RenderSurfaces
          */
        static bool allGLContextsAreShared();

#if 0
        /** Realize the RenderSurface.  When realized, all components of the RenderSurface
          * not already configured are configured, a window and a graphics context are
          * created and made current.  If an already existing graphics context is passed
          * through "sharedGLContext", then the graphics context created will share certain
          * graphics constructs (such as display lists) with "sharedGLContext". */
        bool realize( VisualChooser *vc=NULL, GLContext sharedGLContext=0 );


        void addRealizeCallback( Callback *realizeCB );
        void setRealizeCallback( Callback *realizeCB ) { addRealizeCallback(realizeCB); }

        /** Swaps buffers if RenderSurface quality attribute is DoubleBuffered */
        virtual void swapBuffers(void);

        /** Makes the graphics context and RenderSurface current for rendering */
        bool makeCurrent(void) const;

        /** Where supported, sync() will synchronize with the vertical retrace signal of the
          * graphics display device.  \a divisor specifies the number of vertical retace signals
          * to allow before returning. */
        virtual void sync( int divisor=1 );

        /** Where supported, getRefreshRate() will return the frequency in hz of the vertical
          * retrace signal of the graphics display device.  If getRefreshRate() returns 0, then
          * the underlying support to get the graphics display device's vertical retrace signal
          * is not present. */
        unsigned int getRefreshRate() const;

        /** Where supported, initThreads will initialize all graphics components for thread
          * safety.  InitThreads() should be called before any other calls to Xlib, or OpenGL
          * are made, and should always be called when multi-threaded environments are intended.          */
        static void initThreads();

        /**
         * Puts the calling thread to sleep until the RenderSurface is realized.  Returns true
         * if for success and false for failure.
         * */
        bool waitForRealize();

        /** fullScreen(flag).  If flag is true, RenderSurface resizes its window to fill the
         * entire screen and turns off the border.  If false, the window is returned to a size
         * previously specified.  If previous state specified a border around the window, a
         * the border is replaced
         * */

        /** positionPointer(x,y) places the pointer at window coordinates x, y.
          */
        void positionPointer( int x, int y );

        /** set/getUseDefaultEsc is deprecated */
        void setUseDefaultEsc( bool flag ) {_useDefaultEsc = flag; }
        bool getUseDefaultEsc() { return _useDefaultEsc; }

        /** map and unmap the window */
        void mapWindow();
        void unmapWindow();
#endif
        void fullScreen( bool flag );
        /** setCustomFullScreenRencangle(x,y,width,height).  Programmer may set a customized
          * rectangle to be interpreted as "fullscreen" when fullscreen(true) is called.  This
          * allows configurations that have large virtual screens that span more than one monitor
          * to define a "local" full screen for each monitor.
          */
        void setCustomFullScreenRectangle( int x, int y, unsigned int width, unsigned int height );
        /** useDefaultFullScreenRetangle().  Sets the application back to using the default
          * screen size as fullscreen rather than the custom full screen rectangle
          */
        void useDefaultFullScreenRectangle();

        /** isFullScreen() returns true if the RenderSurface's window fills the entire screen
         * and has no border. */
        bool isFullScreen() const { return _isFullScreen; }

        // TEMPORARY PBUFFER RENDER TO TEXTURE SUPPORT
        // Temporary PBuffer support for Windows.  Somebody got really
        // confused and mixed up PBuffers with a renderable texture and
        // caused this messy headache.
        // These have no meaning in GLX world.....
        enum BufferType {
            FrontBuffer,
            BackBuffer
        };

        enum RenderToTextureMode {
            RenderToTextureMode_None,
            RenderToRGBTexture,
            RenderToRGBATexture
        };

        enum RenderToTextureTarget {
            Texture1D,
            Texture2D,
            TextureCUBE
        };

        enum RenderToTextureOptions {
            RenderToTextureOptions_Default = 0,
            RequestSpaceForMipMaps  = 1,
            RequestLargestPBuffer   = 2
        };

        enum CubeMapFace {
            PositiveX  = 0,
            NegativeX  = 1,
            PositiveY  = 2,
            NegativeY  = 3,
            PositiveZ  = 4,
            NegativeZ  = 5
        };

        /**
          * Bind PBuffer content to the currently selected texture.
          * This method affects PBuffer surfaces only. */
        void bindPBufferToTexture(BufferType buffer = FrontBuffer) const;

        /**
          * Get the render-to-texture mode (PBuffer drawables only).
          * This method has no effect if it is called after realize() */
        RenderToTextureMode getRenderToTextureMode() const;

        /**
          * Set the render-to-texture mode (PBuffer drawables only). You
          * can pass int values different from the constants defined by
          * RenderToTextureMode, in which case they will be applied
          * directly as parameters to the WGL_TEXTURE_FORMAT attribute.
          * This method has no effect if it is called after realize(). */
        void setRenderToTextureMode(RenderToTextureMode mode);

        /**
          * Get the render-to-texture target (PBuffer drawables only).
          * This method has no effect if it is called after realize(). */
        RenderToTextureTarget getRenderToTextureTarget() const;

        /**
          * Set the render-to-texture target (PBuffer drawables only). You
          * can pass int values different from the constants defined by
          * RenderToTextureTarget, in which case they will be applied
          * directly as parameters to the WGL_TEXTURE_TARGET attribute.
          * This method has no effect if it is called after realize(). */
        void setRenderToTextureTarget(RenderToTextureTarget target);

        /**
          * Get the render-to-texture options (PBuffer drawables only).
          * This method has no effect if it is called after realize(). */
        RenderToTextureOptions getRenderToTextureOptions() const;

        /**
          * Set the render-to-texture options (PBuffer drawables only).
          * You can pass any combination of the constants defined in
          * enum RenderToTextureOptions.
          * This method has no effect if it is called after realize(). */
        void setRenderToTextureOptions(RenderToTextureOptions options);

        /**
          * Get which mipmap level on the target texture will be
          * affected by rendering. */
        int getRenderToTextureMipMapLevel() const;

        /**
          * Select which mipmap level on the target texture will be
          * affected by rendering. This method can be called after the
          * PBuffer has been realized. */
        void setRenderToTextureMipMapLevel(int level);

        /**
          * Get which face on the target cube map texture will be
          * affected by rendering. */
        CubeMapFace getRenderToTextureFace() const;

        /**
          * Select which face on the target cube map texture will be
          * affected by rendering. This method can be called after the
          * PBuffer has been realized. */
        void setRenderToTextureFace(CubeMapFace face);

        // END TEMPORARY PBUFFER RENDER TO TEXTURE SUPPORT

        /**
          * Get the (const) vector of user-defined PBuffer attributes. */
        const std::vector<int> &getPBufferUserAttributes() const;

        /**
          * Get the vector of user-defined PBuffer attributes. This
          * vector will be used to initialize the PBuffer's attribute list.*/
        std::vector<int> &getPBufferUserAttributes();

    private :
#ifdef _X11_IMPLEMENTATION
        int (*__glxGetRefreshRateSGI)(unsigned int *);
        int (*__glxGetVideoSyncSGI)(unsigned int *);
        int (*__glxWaitVideoSyncSGI)(int,int,unsigned int *);
        void testVSync( void );
#endif

#if 0
        bool _checkEvents( Display *);
        void _setBorder( bool flag );
        void _setWindowName( const std::string & );
        void _useCursor(bool);
        void _setCursor(Cursor);
        void _setCursorToDefault();
        void _positionPointer(int x, int y);
        unsigned int _refreshRate;
        void _resizeWindow();
#endif
        bool _overrideRedirectFlag;
    protected :

        virtual ~RenderSurface( void );
        //        void _useOverrideRedirect( bool );

        DrawableType       _drawableType;
        std::string        _hostname;
        int                _displayNum;
        float              _windowLeft, _windowRight,
                           _windowBottom, _windowTop;
        int                _windowX, _windowY;
        unsigned int       _windowWidth, _windowHeight;
        unsigned int       _screenWidth, _screenHeight;
        bool               _useCustomFullScreen;
        int                _customFullScreenOriginX;
        int                _customFullScreenOriginY;
        unsigned int       _customFullScreenWidth;
        unsigned int       _customFullScreenHeight;

        int                _screen;
#if 0
        Display *          _dpy;
        Window             _win;
        Window             _parent;
        unsigned int       _parentWindowHeight;
#endif
        RenderSurface      *_readDrawableRenderSurface;
        bool               _realized;
        osg::ref_ptr< VisualChooser >  _visualChooser;
#if 0
        VisualInfo *       _visualInfo;
        unsigned int       _visualID;
        GLContext          _glcontext;
        GLContext          _sharedGLContext;
        Cursor             _currentCursor;
        Cursor             _nullCursor;
        Cursor             _defaultCursor;
        unsigned int       _frameCount;
#endif
        bool               _decorations;
        bool               _useCursorFlag;
        std::string        _windowName;
        bool               _mayFullScreen;
        bool               _isFullScreen;
        bool               _bindInputRectangleToWindowSize;


        // Temporary "Pbuffer support for Win32
        RenderToTextureMode     _rtt_mode;
        RenderToTextureTarget   _rtt_target;
        RenderToTextureOptions  _rtt_options;
        int                     _rtt_mipmap;
        CubeMapFace             _rtt_face;
        bool                    _rtt_dirty_mipmap;
        bool                    _rtt_dirty_face;

#ifdef _OSX_AGL_IMPLEMENTATION
        GLContext          _windowGlcontext;
        GLContext          _fullscreenGlcontext;
        CFDictionaryRef    _oldDisplayMode;

        bool _captureDisplayOrWindow();
        void _releaseDisplayOrWindow();
#endif

        // user-defined PBuffer attributes
        std::vector<int>        _user_pbattr;


        bool _useConfigEventThread;
        bool _checkOwnEvents;
        bool _useDefaultEsc;

        InputRectangle _inputRectangle;

        //        void _computeScreenSize( unsigned int &width, unsigned int &height ) const;

#if 0
        bool  _createVisualInfo();

        virtual bool  _init();
        virtual void  _fini();
        virtual void  run();

        static  void  _initThreads();
#endif

#ifdef _WIN32_IMPLEMENTATION

        class Client : public Producer::Referenced
        {
            public:
                Client(unsigned long mask);
                unsigned int mask() { return _mask; }
                EventQueue *getQueue() { return q.get(); }
                void queue( ref_ptr<Event> ev );

            private :
                unsigned int _mask;
                Producer::ref_ptr<EventQueue> q;
        };
        std::vector < Producer::ref_ptr<Client> >clients;
        void dispatch( ref_ptr<Event> );

        int _ownWindow;
        int _ownVisualChooser;
        int _ownVisualInfo;

        HDC _hdc;
        HINSTANCE _hinstance;

        BOOL CreateGLWindow(char* title, int width, int height, int bits, bool fullscreenflag);
        void KillGLWindow();

        LONG WINAPI proc( Window, UINT, WPARAM, LPARAM );
        static LONG WINAPI s_proc( Window, UINT, WPARAM, LPARAM );
        static std::map <Window, RenderSurface *>registry;

        /* mouse things */
        int _mx, _my;
        unsigned int _mbutton;
        WNDPROC _oldWndProc;


public:
        EventQueue * selectInput( unsigned int mask );

        // if _parent is set, resize the window to
        // fill the client area of the parent
        void resizeToParent();
#endif

        static bool _shareAllGLContexts;

};

}


#endif

