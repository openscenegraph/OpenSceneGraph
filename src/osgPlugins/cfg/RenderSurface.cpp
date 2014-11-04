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

#include <stdlib.h>
#include <string.h>

#include "RenderSurface.h"

using namespace std;
using namespace osgProducer;

const std::string RenderSurface::defaultWindowName = std::string(" *** Producer::RenderSurface *** ");

const unsigned int RenderSurface::UnknownDimension = 0xFFFFFFFF;
const unsigned int RenderSurface::UnknownAmount = 0xFFFFFFFF;
unsigned int RenderSurface::_numScreens = RenderSurface::UnknownAmount;

bool RenderSurface::_shareAllGLContexts = false;
//GLContext RenderSurface::_globallySharedGLContext  = 0L;

void RenderSurface::shareAllGLContexts(bool flag)
{
    _shareAllGLContexts = flag;
}

bool RenderSurface::allGLContextsAreShared()
{
    return _shareAllGLContexts;
}


const std::string &RenderSurface::getDefaultWindowName()
{
    return defaultWindowName;
}

RenderSurface::RenderSurface( void )
{
    _drawableType    = DrawableType_Window;
    _hostname        = "";
    _displayNum     = 0;
//     _screen          = 0;
    _mayFullScreen   = true;
    _isFullScreen    = true;

    // This used to be #ifdefed for the X11 implementation
    // but the code is pure C++ and should compile anywhere
    // The _dislayNum variable is used by CGL as well.
    char *envptr = getenv( "DISPLAY" );
    if( envptr != NULL && *envptr != 0 )
    {
        size_t p0 = 0;
        size_t p1 = string(envptr).find(":", p0);
        _hostname = string(envptr).substr(p0,p1);
        p0 = p1+1;
        p1 = string(envptr).find(".", p0);

        if( p1 > 0 )
        {
            _displayNum = atoi((string(envptr).substr(p0,p1)).c_str());
            p0 = p1+1;
            p1 = string(envptr).length() - p0;
            if( p1 > 0 )
                _screen = atoi((string(envptr).substr(p0,p1)).c_str());
        }
        else if( p1 < string(envptr).length() )
        {
            p1 = string(envptr).length();
            _displayNum = atoi((string(envptr).substr(p0,p1)).c_str());
            _screen = 0;
        }
    }

    _windowLeft     = 0;
    _windowRight    = 1;
    _windowBottom   = 0;
    _windowTop      = 1;
    _windowX        = 0;
    _windowY        = 0;
    _windowWidth    = UnknownDimension;
    _windowHeight   = UnknownDimension;
    _screenWidth    = UnknownDimension;
    _screenHeight   = UnknownDimension;
    _customFullScreenOriginX = 0;
    _customFullScreenOriginY = 0;
    _customFullScreenWidth   = UnknownDimension;
    _customFullScreenHeight  = UnknownDimension;
    _useCustomFullScreen     = false;

    _readDrawableRenderSurface = 0L;
    _windowName      = defaultWindowName;
    _realized        = false;
    _useConfigEventThread = true;
    _overrideRedirectFlag = false;

    char *override_envptr = getenv( "PRODUCER_OVERRIDE_REDIRECT" );
    if( override_envptr != NULL && *override_envptr != 0 )
    {
        if (strcmp(override_envptr,"true")==0 || strcmp(override_envptr,"True")==0 || strcmp(override_envptr,"TRUE")==0)
        {
            _overrideRedirectFlag = true;
        }
        else
        {
            _overrideRedirectFlag = false;
        }
    }

    _decorations     = true;
    _useCursorFlag   = true;




    _useDefaultEsc = true;
    _checkOwnEvents = true;
    _inputRectangle.set( -1.0,  1.0, -1.0, 1.0 );
    _bindInputRectangleToWindowSize = false;


    _rtt_mode   = RenderToTextureMode_None;
    //_rtt_mode   = RenderToRGBTexture;
    _rtt_target = Texture2D;
    _rtt_options = RenderToTextureOptions_Default;
    _rtt_mipmap = 0;
    _rtt_face = PositiveX;
    _rtt_dirty_mipmap = true;
    _rtt_dirty_face = true;


#ifdef _WIN32_IMPLEMENTATION
    _ownWindow = true;
    _ownVisualChooser = true;
    _ownVisualInfo = true;
    _hinstance = NULL;
    _glcontext = NULL;
    _mx = 0;
    _my = 0;
    _mbutton = 0;
#endif
}

RenderSurface::~RenderSurface( void )
{
}


void RenderSurface::setDrawableType( RenderSurface::DrawableType drawableType )
{
    if( _realized )
    {
        std::cerr << "Warning: RenderSurface::setDrawableType() "
                     "has no effect after RenderSurface has been realized\n";
        return;
    }
    _drawableType = drawableType;
}

RenderSurface::DrawableType RenderSurface::getDrawableType()
{
    return _drawableType;
}

void RenderSurface::setReadDrawable( osgProducer::RenderSurface *rs )
{
    _readDrawableRenderSurface = rs;
#if 0
    if( _realized )
        makeCurrent();
#endif
}

#if 0
void RenderSurface::setParentWindow( Window parent )
{
     _parent = parent;
}

Producer::Window RenderSurface::getParentWindow( void ) const
{
    return _parent;
}
#endif

void RenderSurface::setWindowName( const std::string &name )
{
  //    _setWindowName( name );
    _windowName = name;
}

const std::string &RenderSurface::getWindowName( void ) const
{
    return _windowName;
}

void RenderSurface::setHostName( const std::string &name )
{
    _hostname = name;
}

const std::string &RenderSurface::getHostName( void ) const
{
    return _hostname;
}

#if 0
void RenderSurface::setDisplay( Display *dpy )
{
    _dpy = dpy;
}

Producer::Display *RenderSurface::getDisplay()
{
#ifdef WIN32
    return &_hdc;
#else
    return _dpy;
#endif
}

const Producer::Display *RenderSurface::getDisplay() const
{
#ifdef WIN32
    return &_hdc;
#else
    return _dpy;
#endif
}
#endif

void RenderSurface::setDisplayNum( int num )
{
    _displayNum = num;
}

int  RenderSurface::getDisplayNum( void ) const
{
    return _displayNum;
}

void RenderSurface::setScreenNum( int num )
{
    _screen = num;
}

int  RenderSurface::getScreenNum( void ) const
{
    return _screen;
}

void RenderSurface::setWindowRectangle( int x, int y, unsigned int width, unsigned int height, bool resize)
{
    if( _useCustomFullScreen )
    {
        _windowX = x + _customFullScreenOriginX;
        _windowY = y + _customFullScreenOriginY;
    }
    else
    {
        _windowX = x;
        _windowY = y;
    }
    _windowWidth = width;
    _windowHeight = height;
#ifdef _OSX_AGL_IMPLEMENTATION
    fullScreen(false);                  // STH: this may break other implementations, but it is necessary for OSX
#else
    _isFullScreen    = false;
#endif
#if 0
    if( _realized && resize )
        _resizeWindow();
    else
#endif
        if( _bindInputRectangleToWindowSize == true )
            _inputRectangle.set( 0.0, _windowWidth, 0.0, _windowHeight );
}

void RenderSurface::getWindowRectangle( int &x, int &y, unsigned int &width, unsigned int &height ) const
{
    if( _isFullScreen )
    {
        x = 0;
        y = 0;
        if( _useCustomFullScreen == true )
        {
            width  = _customFullScreenWidth;
            height = _customFullScreenHeight;
        }
        else
        {
            width = _screenWidth;
            height = _screenHeight;
        }
    }
    else
    {
        x      = _windowX;
        y      = _windowY;
        width  = _windowWidth;
        height = _windowHeight;
    }
}

void RenderSurface::useDefaultFullScreenRectangle()
{
    _useCustomFullScreen = false;
}

void RenderSurface::setCustomFullScreenRectangle( int x, int y, unsigned int width, unsigned int height )
{
    _customFullScreenOriginX = x;
    _customFullScreenOriginY = y;
    _customFullScreenWidth   = width;
    _customFullScreenHeight  = height;
    _useCustomFullScreen     = true;

    _windowX += _customFullScreenOriginX;
    _windowY += _customFullScreenOriginY;
}

int RenderSurface::getWindowOriginX() const
{
    if( _isFullScreen )
    {
        if( _useCustomFullScreen == true )
            return _customFullScreenOriginX;
        else
            return 0;
    }
    return _windowX;
}

int RenderSurface::getWindowOriginY() const
{
    if( _isFullScreen )
    {
        if( _useCustomFullScreen == true )
            return _customFullScreenOriginY;
        else
            return 0;
    }
    return _windowY;
}

unsigned int RenderSurface::getWindowWidth() const
{
    if( _isFullScreen )
    {
        if( _useCustomFullScreen == true )
            return _customFullScreenWidth;
        else
            return _screenWidth;
    }
    return _windowWidth;
}

unsigned int RenderSurface::getWindowHeight() const
{
    if( _isFullScreen )
    {
        if( _useCustomFullScreen == true )
            return _customFullScreenHeight;
        else
            return _screenHeight;
    }
    return _windowHeight;
}

void RenderSurface::setInputRectangle( const InputRectangle &inputRectangle )
{
    _inputRectangle = inputRectangle;
}

const RenderSurface::InputRectangle &RenderSurface::getInputRectangle() const
{
    return _inputRectangle;
}

void RenderSurface::bindInputRectangleToWindowSize( bool flag)
{
    _bindInputRectangleToWindowSize = flag;
    if( _bindInputRectangleToWindowSize == true )
        _inputRectangle.set( 0.0, float(_windowWidth), 0.0, float(_windowHeight) );
    else
        _inputRectangle.set( -1.0,  1.0, -1.0, 1.0 );
}


void RenderSurface::useBorder( bool flag )
{
    _decorations = flag;
#if 0
    if( _realized )
          _setBorder(_decorations);
#endif
}

bool RenderSurface::usesBorder()
{
    return _decorations;
}

#if 0
void RenderSurface::useCursor( bool flag )
{
     _useCursor(flag);
}

void RenderSurface::setCursor( Cursor cursor )
{
    _setCursor(cursor);
}

void RenderSurface::setCursorToDefault()
{
    _setCursorToDefault();
}
void RenderSurface::setWindow( const Window win )
{
    if( _realized )
    {
        std::cerr << "RenderSurface::setWindow() - cannot set window after RenderSurface has been realized\n";
        return;
    }
    _win = win;
#ifdef WIN32
    _ownWindow = false;
#endif
}

Producer::Window RenderSurface::getWindow( void ) const
{
    return _win;
}

GLContext RenderSurface::getGLContext( void) const
{
    return _glcontext;
}

void RenderSurface::setGLContext( GLContext glContext )
{
    if( _realized )
    {
        std::cerr << "RenderSurface::setGLContext() - Warning: Must be set before realize()." << std::endl;
        return;
    }

    _glcontext = glContext;
}


bool RenderSurface::isRealized() const
{
    return _realized;
}

void RenderSurface::setVisualInfo( VisualInfo *vi )
{
    if( _realized )
    {
        std::cerr << "RenderSurface::setVisualInfo():Warning - has no effect after RenderSurface has been realized\n";
        return;
    }
    _visualInfo = vi;
#ifdef _WIN32_IMPLEMENTATION
    _ownVisualInfo = false;
#endif
}

VisualInfo *RenderSurface::getVisualInfo( void  )
{
    return _visualInfo;
}

const VisualInfo *RenderSurface::getVisualInfo( void  ) const
{
    return _visualInfo;
}

#endif

void RenderSurface::setVisualChooser( VisualChooser *vc )
{
    if( _realized )
    {
        std::cerr << "RenderSurface::setVisualChooser():Warning - has no effect after RenderSurface has been realized\n";
        return;
    }
    _visualChooser = vc;

#ifdef _WIN32_IMPLEMENTATION
    _ownVisualChooser = false;
#endif
}

VisualChooser *RenderSurface::getVisualChooser( void )
{
    return _visualChooser.get();
}

const VisualChooser *RenderSurface::getVisualChooser( void ) const
{
    return _visualChooser.get();
}


#if 0
void RenderSurface::getScreenSize( unsigned int &width, unsigned int &height )  const
{
    if( _realized )
    {
        if( _useCustomFullScreen == true )
        {
            width  = _customFullScreenWidth;
            height = _customFullScreenHeight;
        }
        else
        {
            width  = _screenWidth;
            height = _screenHeight;
        }
    }
    else
    {
        _computeScreenSize(width, height);
    }
}
#endif

#if 0
void  RenderSurface::useConfigEventThread( bool flag )
{
    if( _realized )
    {
        // This message is annoying.  Cameras that share a render surface will call this after the
        // render surface has been realized, which is valid.
        //std::cerr << "RenderSurface::useConfigEventThread():Warning - has no effect after RenderSurface has been realized\n";
        return;
    }
    _useConfigEventThread = flag;
}

unsigned int RenderSurface::getRefreshRate() const
{
    if( !_realized ) return 0;
    return _refreshRate;
}

void RenderSurface::addRealizeCallback( Callback *realizeCB )
{
    if( _realized )
    {
        std::cerr << "RenderSurface::addRealizeCallback() : Warning.  RenderSurface is already realized.  ignored.\n";
            return;
    }
    _realizeCallbacks.push_back( realizeCB );
}

bool RenderSurface::waitForRealize()
{
    if( _realized ) return true;
    while( _realized == false )
        _realizeBlock->block();
    return true;
}

void RenderSurface::positionPointer( int x, int y )
{
    _positionPointer(x,y);
}

void RenderSurface::initThreads()
{
    _initThreads();
}
#endif

RenderSurface::RenderToTextureMode RenderSurface::getRenderToTextureMode() const
{
    return _rtt_mode;
}


void RenderSurface::setRenderToTextureMode(RenderToTextureMode mode)
{
    _rtt_mode = mode;
}


RenderSurface::RenderToTextureTarget RenderSurface::getRenderToTextureTarget() const
{
    return _rtt_target;
}


void RenderSurface::setRenderToTextureTarget(RenderToTextureTarget target)
{
    _rtt_target = target;
}


RenderSurface::RenderToTextureOptions RenderSurface::getRenderToTextureOptions() const
{
    return _rtt_options;
}


void RenderSurface::setRenderToTextureOptions(RenderToTextureOptions options)
{
    _rtt_options = options;
}

int RenderSurface::getRenderToTextureMipMapLevel() const
{
    return _rtt_mipmap;
}


void RenderSurface::setRenderToTextureMipMapLevel(int level)
{
    _rtt_mipmap = level;
    _rtt_dirty_mipmap = true;
}

RenderSurface::CubeMapFace RenderSurface::getRenderToTextureFace() const
{
    return _rtt_face;
}


void RenderSurface::setRenderToTextureFace(CubeMapFace face)
{
    _rtt_face = face;
    _rtt_dirty_face = true;
}

const std::vector<int> &RenderSurface::getPBufferUserAttributes() const
{
    return _user_pbattr;
}

std::vector<int> &RenderSurface::getPBufferUserAttributes()
{
    return _user_pbattr;
}



void RenderSurface::useOverrideRedirect(bool flag)
{
    _overrideRedirectFlag = flag;
}

bool RenderSurface::usesOverrideRedirect()
{
    return _overrideRedirectFlag;
}

