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

#if defined (__APPLE__) && (!__LP64__)

#include <osg/observer_ptr>

#include <osgViewer/api/Carbon/PixelBufferCarbon>
#include <osgViewer/api/Carbon/GraphicsWindowCarbon>

#include <osg/DeleteHandler>

#include <Carbon/Carbon.h>
#include <OpenGL/OpenGL.h>

#include <iostream>

#include "DarwinUtils.h"

using namespace osgViewer;
using namespace osgDarwin;


// Carbon-Eventhandler to handle the click in the close-widget and the resize of windows

static pascal OSStatus GraphicsWindowEventHandler(EventHandlerCallRef nextHandler, EventRef event, void* userData)
{
    WindowRef           window;
    Rect                bounds;
    OSStatus            result = eventNotHandledErr; /* report failure by default */

    OSG_INFO << "GraphicsWindowEventHandler" << std::endl;

    GraphicsWindowCarbon* w = (GraphicsWindowCarbon*)userData;
    if (!w)
        return result;

    GetEventParameter(event, kEventParamDirectObject, typeWindowRef, NULL,
                         sizeof(window), NULL, &window);

    switch(GetEventClass(event))
    {
        case kEventClassTablet:
        case kEventClassMouse:
            if (w->handleMouseEvent(event))
                result = noErr;
            break;

        case kEventClassKeyboard:
            if (w->handleKeyboardEvent(event))
                result = noErr;
            break;

        case kEventClassWindow: {

            switch (GetEventKind(event))
                {
                    case kEventWindowBoundsChanging:
                        // left the code for live-resizing, but it is not used, because of window-refreshing issues...
                        GetEventParameter( event, kEventParamCurrentBounds, typeQDRectangle, NULL, sizeof(Rect), NULL, &bounds );

                        w->adaptResize(bounds.left, bounds.top, bounds.right - bounds.left, bounds.bottom - bounds.top);
                        w->requestRedraw();
                        result = noErr;
                        break;

                    case kEventWindowBoundsChanged:
                        InvalWindowRect(window, GetWindowPortBounds(window, &bounds));
                        GetWindowBounds(window, kWindowContentRgn, &bounds);
                        w->adaptResize(bounds.left, bounds.top, bounds.right - bounds.left, bounds.bottom - bounds.top);
                        result = noErr;
                        break;

                    case kEventWindowClose:
                        w->requestClose();
                        result = noErr;
                        break;

                    default:
                        break;
                }
            }
        default:
            //std::cout << "unknown: " << GetEventClass(event) << std::endl;
            break;
    }

    //if (result == eventNotHandledErr)
    //    result = CallNextEventHandler (nextHandler, event);

    return result;
}


static bool s_quit_requested = false;

// Application eventhandler -- listens for a quit-event
static pascal OSStatus ApplicationEventHandler(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
{

    HICommand commandStruct;

    OSErr  err = eventNotHandledErr;

    GetEventParameter (inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &commandStruct);

    switch(commandStruct.commandID) {
        case kHICommandQuit:
            s_quit_requested = true;
            err = noErr;
            break;

    }

    return err;
}

// AppleEventHandler, listens to the Quit-AppleEvent
static pascal OSErr QuitAppleEventHandler(const AppleEvent *theAppleEvent, AppleEvent *reply, long handlerRefcon ) {
    s_quit_requested = true;
    return (noErr);
}


namespace osgViewer
{

// small helper class which maps the raw key codes to osgGA::GUIEventAdapter::Keys

class CarbonKeyboardMap {

    public:
        CarbonKeyboardMap()
        {
            _keymap[53                ] =  osgGA::GUIEventAdapter::KEY_Escape;
            _keymap[115                ] =  osgGA::GUIEventAdapter::KEY_Home;
            _keymap[76                ] =  osgGA::GUIEventAdapter::KEY_KP_Enter;
            _keymap[119                ] =  osgGA::GUIEventAdapter::KEY_End;
            _keymap[36                ] =  osgGA::GUIEventAdapter::KEY_Return;
            _keymap[116                ] =  osgGA::GUIEventAdapter::KEY_Page_Up;
            _keymap[121                ] = osgGA::GUIEventAdapter::KEY_Page_Down;
            _keymap[123                ] = osgGA::GUIEventAdapter::KEY_Left;
            _keymap[124                ] = osgGA::GUIEventAdapter::KEY_Right;
            _keymap[126                ] = osgGA::GUIEventAdapter::KEY_Up;
            _keymap[125                ] = osgGA::GUIEventAdapter::KEY_Down;
            _keymap[51                ] = osgGA::GUIEventAdapter::KEY_BackSpace;
            _keymap[48                ] = osgGA::GUIEventAdapter::KEY_Tab;
            _keymap[49                ] = osgGA::GUIEventAdapter::KEY_Space;
            _keymap[117                ] = osgGA::GUIEventAdapter::KEY_Delete;

            _keymap[122                    ] = osgGA::GUIEventAdapter::KEY_F1;
            _keymap[120                    ] = osgGA::GUIEventAdapter::KEY_F2;
            _keymap[99                    ] = osgGA::GUIEventAdapter::KEY_F3;
            _keymap[118                    ] = osgGA::GUIEventAdapter::KEY_F4;
            _keymap[96                    ] = osgGA::GUIEventAdapter::KEY_F5;
            _keymap[97                    ] = osgGA::GUIEventAdapter::KEY_F6;
            _keymap[98                    ] = osgGA::GUIEventAdapter::KEY_F7;
            _keymap[100                    ] = osgGA::GUIEventAdapter::KEY_F8;
            _keymap[101                    ] = osgGA::GUIEventAdapter::KEY_F9;
            _keymap[109                    ] = osgGA::GUIEventAdapter::KEY_F10;
            _keymap[103                    ] = osgGA::GUIEventAdapter::KEY_F11;
            _keymap[111                    ] = osgGA::GUIEventAdapter::KEY_F12;

            _keymap[75                    ] = osgGA::GUIEventAdapter::KEY_KP_Divide;
            _keymap[67                    ] = osgGA::GUIEventAdapter::KEY_KP_Multiply;
            _keymap[78                    ] = osgGA::GUIEventAdapter::KEY_KP_Subtract;
            _keymap[69                    ] = osgGA::GUIEventAdapter::KEY_KP_Add;
            _keymap[89                    ] = osgGA::GUIEventAdapter::KEY_KP_Home;
            _keymap[91                    ] = osgGA::GUIEventAdapter::KEY_KP_Up;
            _keymap[92                    ] = osgGA::GUIEventAdapter::KEY_KP_Page_Up;
            _keymap[86                    ] = osgGA::GUIEventAdapter::KEY_KP_Left;
            _keymap[87                    ] = osgGA::GUIEventAdapter::KEY_KP_Begin;
            _keymap[88                    ] = osgGA::GUIEventAdapter::KEY_KP_Right;
            _keymap[83                    ] = osgGA::GUIEventAdapter::KEY_KP_End;
            _keymap[84                    ] = osgGA::GUIEventAdapter::KEY_KP_Down;
            _keymap[85                    ] = osgGA::GUIEventAdapter::KEY_KP_Page_Down;
            _keymap[82                    ] = osgGA::GUIEventAdapter::KEY_KP_Insert;
            _keymap[65                    ] = osgGA::GUIEventAdapter::KEY_KP_Delete;

        }

        ~CarbonKeyboardMap() {
        }

        unsigned int remapKey(unsigned int key, unsigned int rawkey)
        {
            KeyMap::iterator itr = _keymap.find(rawkey);
            if (itr == _keymap.end()) return key;
            else return itr->second;
        }
    private:
        typedef std::map<unsigned int, osgGA::GUIEventAdapter::KeySymbol> KeyMap;
        KeyMap _keymap;
};

/** remaps a native os x keycode to a GUIEventAdapter-keycode */
static unsigned int remapCarbonKey(unsigned int key, unsigned int rawkey)
{
    static CarbonKeyboardMap s_CarbonKeyboardMap;
    return s_CarbonKeyboardMap.remapKey(key,rawkey);
}


class CarbonWindowAdapter : public MenubarController::WindowAdapter {
public:
    CarbonWindowAdapter(GraphicsWindowCarbon* win) : MenubarController::WindowAdapter(), _win(win) {}
    virtual bool valid() {return (_win.valid() && _win->valid()); }
    virtual void getWindowBounds(CGRect& rect)
    {
        Rect windowBounds;
        OSErr error = GetWindowBounds(_win->getNativeWindowRef(), kWindowStructureRgn, &windowBounds);
        rect.origin.x = windowBounds.left;
        rect.origin.y = windowBounds.top;
        rect.size.width = windowBounds.right - windowBounds.left;
        rect.size.height = windowBounds.bottom - windowBounds.top;
    }

    osgViewer::GraphicsWindow* getWindow()  { return _win.get(); }
private:
    osg::observer_ptr<GraphicsWindowCarbon> _win;
};



void GraphicsWindowCarbon::init()
{
    if (_initialized) return;

    // getEventQueue()->setCurrentEventState(osgGA::GUIEventAdapter::getAccumulatedEventState().get());

    _lastModifierKeys = 0;
    _windowTitleHeight = 0;
    _closeRequested = false;
    _ownsWindow = false;
    _context = NULL;
    _window = NULL;
    _pixelFormat = PixelBufferCarbon::createPixelFormat(_traits.get());
    if (!_pixelFormat)
    {
        OSG_WARN << "GraphicsWindowCarbon::init could not create a valid pixelformat" << std::endl;
    }
    _valid = (_pixelFormat != NULL);
    _initialized = true;
    
    // make sure the event queue has the correct window rectangle size and input range
    getEventQueue()->syncWindowRectangleWithGraphcisContext();
}

bool GraphicsWindowCarbon::setWindowDecorationImplementation(bool flag)
{
    _useWindowDecoration = flag;

    if (_realized)
    {
        OSErr err = noErr;
        Rect bounds;
        GetWindowBounds(getNativeWindowRef(), kWindowContentRgn, &bounds);

        if (_useWindowDecoration)
        {
            err = ChangeWindowAttributes(getNativeWindowRef(),  kWindowStandardDocumentAttributes,  kWindowNoTitleBarAttribute | kWindowNoShadowAttribute);
            SetWindowBounds(getNativeWindowRef(), kWindowContentRgn, &bounds);
        }
        else
        {
            err = ChangeWindowAttributes(getNativeWindowRef(), kWindowNoTitleBarAttribute | kWindowNoShadowAttribute, kWindowStandardDocumentAttributes);
            SetWindowBounds(getNativeWindowRef(), kWindowContentRgn, &bounds);
        }

        if (err != noErr)
        {
            OSG_WARN << "GraphicsWindowCarbon::setWindowDecoration failed with " << err << std::endl;
            return false;
        }

        // update titlebar-height
        Rect titleRect;
        GetWindowBounds(_window, kWindowTitleBarRgn, &titleRect);
        _windowTitleHeight = abs(titleRect.bottom - titleRect.top);

        // sth: I don't know why I have to reattach the context to the window here, If I don't do this  I get blank areas, where the titlebar was.
        // InvalWindowRect doesn't help here :-/

        aglSetDrawable(_context, 0);
        aglSetDrawable(_context, GetWindowPort(_window));

        MenubarController::instance()->update();
    }

    return true;
}


WindowAttributes GraphicsWindowCarbon::computeWindowAttributes(bool useWindowDecoration, bool supportsResize) {
    WindowAttributes attr;

    if (useWindowDecoration)
    {
        if (supportsResize)
            attr = (kWindowStandardDocumentAttributes | kWindowStandardHandlerAttribute);
        else
            attr = (kWindowStandardDocumentAttributes | kWindowStandardHandlerAttribute) & ~kWindowResizableAttribute;
    }
    else
    {
        attr = kWindowNoTitleBarAttribute | kWindowNoShadowAttribute | kWindowStandardHandlerAttribute;
        if (supportsResize)
            attr |= kWindowResizableAttribute;
    }
    return attr;
}

void GraphicsWindowCarbon::installEventHandler() {

    // register window event handler to receive resize-events
    EventTypeSpec   windEventList[] = {
        { kEventClassWindow, kEventWindowBoundsChanged},
        { kEventClassWindow, kEventWindowClose},

        {kEventClassMouse, kEventMouseDown},
        {kEventClassMouse, kEventMouseUp},
        {kEventClassMouse, kEventMouseMoved},
        {kEventClassMouse, kEventMouseDragged},
        {kEventClassMouse, kEventMouseWheelMoved},
        {kEventClassMouse, 11 /* kEventMouseScroll */},

        {kEventClassKeyboard, kEventRawKeyDown},
        {kEventClassKeyboard, kEventRawKeyRepeat},
        {kEventClassKeyboard, kEventRawKeyUp},
        {kEventClassKeyboard, kEventRawKeyModifiersChanged},
        {kEventClassKeyboard, kEventHotKeyPressed},
        {kEventClassKeyboard, kEventHotKeyReleased},
    };

    InstallWindowEventHandler(_window, NewEventHandlerUPP(GraphicsWindowEventHandler),  GetEventTypeCount(windEventList), windEventList, this, NULL);
 }


bool GraphicsWindowCarbon::realizeImplementation()
{
    if (!_initialized) init();
    if (!_initialized) return false;
    if (!_traits) return false;

    OSG_INFO << "GraphicsWindowCarbon::realizeImplementation" << std::endl;

    setWindowDecoration(_traits->windowDecoration);
    useCursor(_traits->useCursor);

    // move the window to the right screen
    DarwinWindowingSystemInterface* wsi = dynamic_cast<DarwinWindowingSystemInterface*>(osg::GraphicsContext::getWindowingSystemInterface());
    int screenLeft = 0, screenTop = 0;
    if (wsi)
    {
        wsi->getScreenTopLeft((*_traits), screenLeft, screenTop);
    }

    WindowData *windowData = ( _traits.get() && _traits->inheritedWindowData.get() ) ? static_cast<osgViewer::GraphicsWindowCarbon::WindowData*>(_traits->inheritedWindowData.get()) : 0;

    _ownsWindow = (windowData) ? (windowData->getNativeWindowRef() == NULL) : true;

    if (_ownsWindow) {

        // create the window
        Rect bounds = {_traits->y + screenTop, _traits->x + screenLeft, _traits->y + _traits->height + screenTop, _traits->x + _traits->width + screenLeft};
        OSStatus err = 0;
        WindowAttributes attr = computeWindowAttributes(_useWindowDecoration, _traits->supportsResize);

        err = CreateNewWindow(kDocumentWindowClass, attr, &bounds, &_window);

        if (err) {
            OSG_WARN << "GraphicsWindowCarbon::realizeImplementation: failed to create window: " << err << std::endl;
            return false;
        } else {
            OSG_INFO << "GraphicsWindowCarbon::realizeImplementation: window created with bounds(" << bounds.top << ", " << bounds.left << ", " << bounds.bottom << ", " << bounds.right << ")" << std::endl;
        }
    }
    else {
         _window = windowData->getNativeWindowRef();
    }

    Rect titleRect;
    GetWindowBounds(_window, kWindowTitleBarRgn, &titleRect);
    _windowTitleHeight = abs(titleRect.bottom - titleRect.top);

    if ((_ownsWindow) || (windowData && windowData->installEventHandler()))
        installEventHandler();

    // set the window title
    setWindowName(_traits->windowName);

    // create the context
    AGLContext sharedContextCarbon = NULL;

    GraphicsHandleCarbon* graphicsHandleCarbon = dynamic_cast<GraphicsHandleCarbon*>(_traits->sharedContext.get());
    if (graphicsHandleCarbon)
    {
        sharedContextCarbon = graphicsHandleCarbon->getAGLContext();
    }

    _context = aglCreateContext (_pixelFormat, sharedContextCarbon);
    if (!_context) {
        OSG_WARN << "GraphicsWindowCarbon::realizeImplementation: failed to create context: " << aglGetError() << std::endl;
        return false;
    }


    if ( windowData && windowData->getAGLDrawable() ) {
        aglSetDrawable(_context, (AGLDrawable)*(windowData->getAGLDrawable()) );

    } else {
        aglSetDrawable(_context, GetWindowPort(_window));
        ShowWindow(_window);
        MenubarController::instance()->attachWindow( new CarbonWindowAdapter(this) );
    }

    makeCurrent();

    if ((_traits->useMultiThreadedOpenGLEngine) && (OpenThreads::GetNumberOfProcessors() > 1)) {
        // enable Multi-threaded OpenGL Execution:
        CGLError cgerr = kCGLNoError;
        CGLContextObj ctx = CGLGetCurrentContext();

#if 0
        cgerr =  CGLEnable( ctx, kCGLCEMPEngine);
#else
        // the above use of kCGLCEMPEngine is not backwards compatible
        // so we'll use the raw value of it to keep things compiling on older
        // versions of OSX.
        cgerr =  CGLEnable( ctx, static_cast <CGLContextEnable>(313) );
#endif
        if (cgerr != kCGLNoError )
        {
            OSG_INFO << "GraphicsWindowCarbon::realizeImplementation: multi-threaded OpenGL Execution not available" << std::endl;
        }
    }

    InitCursor();

    // enable vsync
    if (_traits->vsync) {
        GLint swap = 1;
        aglSetInteger (_context, AGL_SWAP_INTERVAL, &swap);
    }
    _currentVSync = _traits->vsync;

    _realized = true;

    // make sure the event queue has the correct window rectangle size and input range
    getEventQueue()->syncWindowRectangleWithGraphcisContext();
    
    return _realized;
}



bool GraphicsWindowCarbon::makeCurrentImplementation()
{

    return (aglSetCurrentContext(_context) == GL_TRUE);
}

bool GraphicsWindowCarbon::releaseContextImplementation()
{
    if (!_realized)
    {
        OSG_NOTICE<<"Warning: GraphicsWindow not realized, cannot do makeCurrent."<<std::endl;
        return false;
    }

    // OSG_NOTICE<<"makeCurrentImplementation "<<this<<" "<<OpenThreads::Thread::CurrentThread()<<std::endl;
    // OSG_NOTICE<<"   glXMakeCurrent ("<<_display<<","<<_window<<","<<_glxContext<<std::endl;
    return (aglSetCurrentContext(NULL) == GL_TRUE);
}



void GraphicsWindowCarbon::closeImplementation()
{
    // OSG_INFO << "GraphicsWindowCarbon::closeImplementation" << std::endl;
    _valid = false;
    _realized = false;

    // there's a possibility that the MenubarController is destructed already, so prevent a crash:
    MenubarController* mbc = MenubarController::instance();
    if (mbc) mbc->detachWindow(this);

    if (_pixelFormat)
    {
        aglDestroyPixelFormat(_pixelFormat);
        _pixelFormat = NULL;
    }

    if (_context)
    {
        aglSetDrawable(_context, NULL);
        aglSetCurrentContext(NULL);
        aglDestroyContext(_context);
        _context = NULL;
    }

    if (_ownsWindow && _window) DisposeWindow(_window);
    _window = NULL;
}



void GraphicsWindowCarbon::swapBuffersImplementation()
{
    // check for vsync change
    if (_traits.valid() && _traits->vsync != _currentVSync)
    {
        const bool on = _traits->vsync;
        GLint swap = (on ? 1 : 0);
        aglSetInteger (_context, AGL_SWAP_INTERVAL, &swap);
        OSG_NOTICE << "GraphicsWindowCarbon: VSync=" << (on ? "on" : "off") << std::endl;
        _currentVSync = on;
    }

    aglSwapBuffers(_context);
}



void GraphicsWindowCarbon::resizedImplementation(int x, int y, int width, int height)
{
    GraphicsContext::resizedImplementation(x, y, width, height);

    aglUpdateContext(_context);
    MenubarController::instance()->update();

    getEventQueue()->windowResize(x,y,width, height, getEventQueue()->getTime());
}



bool GraphicsWindowCarbon::handleMouseEvent(EventRef theEvent)
{

    static unsigned int lastEmulatedMouseButton = 0;
    // mouse down event
    Point wheresMyMouse;
    GetEventParameter (theEvent, kEventParamWindowMouseLocation, typeQDPoint, NULL, sizeof(wheresMyMouse), NULL, &wheresMyMouse);

    wheresMyMouse.v -= _windowTitleHeight;
    if (_useWindowDecoration && (wheresMyMouse.v < 0))
        return false;

    Point wheresMyMouseGlobal;
    GetEventParameter (theEvent, kEventParamMouseLocation, typeQDPoint, NULL, sizeof(wheresMyMouse), NULL, &wheresMyMouseGlobal);

    EventMouseButton mouseButton = 0;
    GetEventParameter (theEvent, kEventParamMouseButton, typeMouseButton, NULL, sizeof(mouseButton), NULL, &mouseButton);

    UInt32 modifierKeys;
    GetEventParameter (theEvent,kEventParamKeyModifiers,typeUInt32, NULL,sizeof(modifierKeys), NULL,&modifierKeys);


    WindowRef win;
    int fwres = FindWindow(wheresMyMouseGlobal, &win);
    // return false when Window is inactive; For enabling click-to-active on window by delegating event to default handler
    if (((fwres != inContent) && (fwres > 0) && (mouseButton >= 1)) || !IsWindowActive(win))
    {
        return false;
    }
    else
    {
        UInt32 clickCount;
        GetEventParameter(theEvent, kEventParamClickCount, typeUInt32, NULL, sizeof(clickCount), NULL, &clickCount);
        // swap right and middle buttons so that middle button is 2, right button is 3.
        if (mouseButton==3) mouseButton = 2;
        else if (mouseButton==2) mouseButton = 3;

        // check tablet pointer device and map it to a mouse button
        TabletProximityRec    theTabletRecord;    // The Tablet Proximity Record
        // Extract the Tablet Proximity reccord from the event.
        if(noErr == GetEventParameter(theEvent, kEventParamTabletProximityRec,
                                      typeTabletProximityRec, NULL,
                                      sizeof(TabletProximityRec),
                                      NULL, (void *)&theTabletRecord))
        {
            osgGA::GUIEventAdapter::TabletPointerType pointerType;
            switch(theTabletRecord.pointerType)
            {
                case 1: // pen
                    pointerType = osgGA::GUIEventAdapter::PEN;
                    break;

                case 2: // puck
                    pointerType = osgGA::GUIEventAdapter::PUCK;
                    break;

                case 3: // eraser
                    pointerType = osgGA::GUIEventAdapter::ERASER;
                    break;

                default:
                   pointerType = osgGA::GUIEventAdapter::UNKNOWN;
                   break;
            }

            getEventQueue()->penProximity(pointerType, (theTabletRecord.enterProximity != 0));
        }

        // get tilt and rotation from the pen
        TabletPointRec theTabletPointRecord;
        if(noErr == GetEventParameter(theEvent,  kEventParamTabletPointRec, typeTabletPointRec, NULL,
                sizeof(TabletPointRec), NULL, (void *)&theTabletPointRecord))
        {
            int penRotation = (int)theTabletPointRecord.rotation * 9 / 575; //to get angle between 0 to 360 grad
            penRotation = -(((penRotation + 180) % 360) - 180) ;          //for same range on all plattforms we need -180 to 180
            getEventQueue()->penOrientation (
                    theTabletPointRecord.tiltX * 60 / 32767.0f,  //multiply with 60 to get angle between -60 to 60 grad
                    -theTabletPointRecord.tiltY * 60 / 32767.0f,  //multiply with 60 to get angle between -60 to 60 grad
                    penRotation
            );
        }

        switch(GetEventKind(theEvent))
        {
            case kEventMouseDown:
                {
                    float mx = wheresMyMouse.h;
                    float my = wheresMyMouse.v;
                    transformMouseXY(mx, my);

                    lastEmulatedMouseButton = 0;

                    if (mouseButton == 1)
                    {
                        if( modifierKeys & cmdKey )
                        {
                            mouseButton = lastEmulatedMouseButton = 3;
                        }
                        else if( modifierKeys & optionKey )
                        {
                            mouseButton = lastEmulatedMouseButton = 2;
                        }
                    }

                    if (clickCount > 1)
                        getEventQueue()->mouseDoubleButtonPress(mx,my, mouseButton);
                    else
                        getEventQueue()->mouseButtonPress(mx, my, mouseButton);
                }
                break;
            case kEventMouseUp:
                {
                    float mx = wheresMyMouse.h;
                    float my = wheresMyMouse.v;
                    transformMouseXY(mx, my);
                    if (lastEmulatedMouseButton > 0) {
                        getEventQueue()->mouseButtonRelease(mx, my, lastEmulatedMouseButton);
                        lastEmulatedMouseButton = 0;
                    }
                    else {
                        getEventQueue()->mouseButtonRelease(mx, my, mouseButton);
                    }
                }
                break;

            case kEventMouseDragged:
                {
                    // get pressure from the pen, only when mouse/pen is dragged
                    TabletPointRec    theTabletRecord;
                    if(noErr == GetEventParameter(theEvent,  kEventParamTabletPointRec, typeTabletPointRec, NULL,
                                    sizeof(TabletPointRec), NULL, (void *)&theTabletRecord)) {

                        getEventQueue()->penPressure(theTabletRecord.pressure / 65535.0f);
                    }

                    float mx = wheresMyMouse.h;
                    float my = wheresMyMouse.v;
                    transformMouseXY(mx, my);
                    getEventQueue()->mouseMotion(mx, my);
                }
                break;

            case kEventMouseMoved:
                {
                    float mx = wheresMyMouse.h;
                    float my = wheresMyMouse.v;
                    transformMouseXY(mx, my);
                    getEventQueue()->mouseMotion(mx, my);
                }
                break;

            // mouse with scroll-wheels
            case kEventMouseWheelMoved:
                {
                    EventMouseWheelAxis axis;
                    SInt32 delta;
                    if (noErr == GetEventParameter( theEvent, kEventParamMouseWheelAxis, typeMouseWheelAxis, NULL, sizeof(axis), NULL, &axis )) {
                        if (noErr == GetEventParameter( theEvent, kEventParamMouseWheelDelta, typeLongInteger, NULL, sizeof(delta), NULL, &delta )) {
                            switch (axis) {
                                case kEventMouseWheelAxisX:
                                    getEventQueue()->mouseScroll( (delta > 0) ? osgGA::GUIEventAdapter::SCROLL_RIGHT : osgGA::GUIEventAdapter::SCROLL_LEFT);
                                    break;
                                case kEventMouseWheelAxisY:
                                    getEventQueue()->mouseScroll( (delta < 0) ? osgGA::GUIEventAdapter::SCROLL_DOWN : osgGA::GUIEventAdapter::SCROLL_UP);
                                    break;
                            }
                        }
                    }
                }
                break;

            // new trackpads and mighty mouse, (not officially documented, see http://developer.apple.com/qa/qa2005/qa1453.html )
            case 11:
                {
                    enum
                    {
                        kEventParamMouseWheelSmoothVerticalDelta       = 'saxy', // typeSInt32
                        kEventParamMouseWheelSmoothHorizontalDelta     = 'saxx' // typeSInt32
                    };

                    SInt32 scroll_delta_x = 0;
                    SInt32 scroll_delta_y = 0;
                    OSErr err = noErr;
                    err = GetEventParameter( theEvent, kEventParamMouseWheelSmoothVerticalDelta, typeLongInteger, NULL, sizeof(scroll_delta_y), NULL, &scroll_delta_y );
                    err = GetEventParameter( theEvent, kEventParamMouseWheelSmoothHorizontalDelta, typeLongInteger, NULL, sizeof(scroll_delta_x), NULL, &scroll_delta_x );

                    if ((scroll_delta_x != 0) || (scroll_delta_y != 0)) {
                        getEventQueue()->mouseScroll2D( scroll_delta_x, scroll_delta_y);
                    }
                }
                break;

            default:
                return false;
        }
    }

    return true;
}



bool GraphicsWindowCarbon::handleKeyboardEvent(EventRef theEvent)
{
    handleModifierKeys(theEvent);

    OSStatus status;

    UInt32 rawkey;
    GetEventParameter (theEvent,kEventParamKeyCode,typeUInt32, NULL,sizeof(rawkey), NULL,&rawkey);

    // OSG_INFO << "key code: " << rawkey << " modifiers: " << modifierKeys << std::endl;

    UInt32 dataSize;
    /* jbw check return status so that we don't allocate a huge array */
    status = GetEventParameter( theEvent, kEventParamKeyUnicodes, typeUnicodeText, NULL, 0, &dataSize, NULL );
    if (status != noErr) return false;
    if (dataSize<=1) return false;

    UniChar* uniChars = new UniChar[dataSize+1];
    GetEventParameter( theEvent, kEventParamKeyUnicodes, typeUnicodeText, NULL, dataSize, NULL, (void*)uniChars );

    unsigned int keychar = remapCarbonKey(static_cast<unsigned long>(uniChars[0]), rawkey);

    switch(GetEventKind(theEvent))
    {
        case kEventRawKeyDown:
        case kEventRawKeyRepeat:
        {
            //OSG_INFO << "GraphicsWindowCarbon::keyPress Up" << std::endl;
            //getEventQueue()->getCurrentEventState()->setModKeyMask(modifierMask);
            //OSG_INFO << "GraphicsWindowCarbon::keyPress" << std::endl;
            getEventQueue()->keyPress(keychar);
            break;
        }

        case kEventRawKeyUp:
        {
            //OSG_INFO << "GraphicsWindowCarbon::keyPress" << std::endl;
            //getEventQueue()->getCurrentEventState()->setModKeyMask(modifierMask);
            getEventQueue()->keyRelease(keychar);
            break;
        }

        default:
             break;

    }

    delete[] uniChars;

    return true;
}

void GraphicsWindowCarbon::handleModifierKey(UInt32 modifierKey, UInt32 modifierMask, osgGA::GUIEventAdapter::KeySymbol keySymbol) {

    if ((modifierKey & modifierMask) && !(_lastModifierKeys & modifierMask))
    {
        getEventQueue()->keyPress(keySymbol);
    }

    if (!(modifierKey & modifierMask) && (_lastModifierKeys & modifierMask))
    {
        getEventQueue()->keyRelease(keySymbol);
    }
}

bool GraphicsWindowCarbon::handleModifierKeys(EventRef theEvent)
{
    UInt32 modifierKeys;
    GetEventParameter (theEvent,kEventParamKeyModifiers,typeUInt32, NULL,sizeof(modifierKeys), NULL,&modifierKeys);

    //std::cout << modifierKeys << std::endl;
    if (_lastModifierKeys == modifierKeys)
        return false;

    handleModifierKey(modifierKeys, shiftKey, osgGA::GUIEventAdapter::KEY_Shift_L);
    handleModifierKey(modifierKeys, controlKey, osgGA::GUIEventAdapter::KEY_Control_L);
    handleModifierKey(modifierKeys, optionKey, osgGA::GUIEventAdapter::KEY_Alt_L);
    handleModifierKey(modifierKeys, cmdKey, osgGA::GUIEventAdapter::KEY_Super_L);

    // Caps lock needs some special handling, i did not find a way to get informed when the caps-lock-key gets released
    if ((modifierKeys & alphaLock) && !(_lastModifierKeys & alphaLock))
    {
        getEventQueue()->keyPress(osgGA::GUIEventAdapter::KEY_Caps_Lock);
        getEventQueue()->keyRelease(osgGA::GUIEventAdapter::KEY_Caps_Lock);
    }

    if (!(modifierKeys & alphaLock) && (_lastModifierKeys & alphaLock))
    {
        getEventQueue()->keyPress(osgGA::GUIEventAdapter::KEY_Caps_Lock);
        getEventQueue()->keyRelease(osgGA::GUIEventAdapter::KEY_Caps_Lock);
    }

    _lastModifierKeys = modifierKeys;
    return true;
}



bool GraphicsWindowCarbon::checkEvents()
{
    if (!_realized) return false;

    EventRef theEvent;
    EventTargetRef theTarget = GetEventDispatcherTarget();
    while (ReceiveNextEvent(0, NULL, 0,true, &theEvent)== noErr)
    {
        switch(GetEventClass(theEvent))
        {
            case kEventClassMouse:
                    {
                    // handle the menubar
                    Point wheresMyMouse;
                    GetEventParameter (theEvent, kEventParamMouseLocation, typeQDPoint, NULL, sizeof(wheresMyMouse), NULL, &wheresMyMouse);

                    EventMouseButton mouseButton = 0;
                    GetEventParameter (theEvent, kEventParamMouseButton, typeMouseButton, NULL, sizeof(mouseButton), NULL, &mouseButton);

                    WindowRef win;
                    int fwres = FindWindow(wheresMyMouse, &win);

                    if ((fwres == inMenuBar) && (mouseButton >= 1)) {
                        MenuSelect(wheresMyMouse);
                        HiliteMenu(0);
                        return !(getEventQueue()->empty());
                    }
                    break;
                }

            case kEventClassApplication:
                switch (GetEventKind(theEvent)) {
                    case kEventAppQuit:
                        getEventQueue()->quitApplication();
                        break;
                }
                break;

            case kEventClassAppleEvent:
                {
                    EventRecord eventRecord;
                    ConvertEventRefToEventRecord(theEvent, &eventRecord);
                    AEProcessAppleEvent(&eventRecord);
                    return;
                }
                break;

        }
        SendEventToEventTarget (theEvent, theTarget);
        ReleaseEvent(theEvent);
    }
    if (_closeRequested)
        getEventQueue()->closeWindow();

    if (s_quit_requested) {
        getEventQueue()->quitApplication();
        s_quit_requested = false;
    }

    return !(getEventQueue()->empty());
}


bool GraphicsWindowCarbon::setWindowRectangleImplementation(int x, int y, int width, int height)
{
    int screenLeft(0), screenTop(0);
    DarwinWindowingSystemInterface* wsi = dynamic_cast<DarwinWindowingSystemInterface*>(osg::GraphicsContext::getWindowingSystemInterface());
   if (wsi)
    {
        wsi->getScreenTopLeft((*_traits), screenLeft, screenTop);
    }

    Rect bounds = {y + screenTop, x + screenLeft, y + height + screenTop, x + width + screenLeft};
    SetWindowBounds(getNativeWindowRef(), kWindowContentRgn, &bounds);
    aglUpdateContext(_context);
    MenubarController::instance()->update();
    return true;
}



void GraphicsWindowCarbon::adaptResize(int x, int y, int w, int h)
{
    DarwinWindowingSystemInterface* wsi = dynamic_cast<DarwinWindowingSystemInterface*>(osg::GraphicsContext::getWindowingSystemInterface());
    int screenLeft(0), screenTop(0);
    if (wsi) {

        // get the screen containing the window
        unsigned int screenNdx = wsi->getScreenContaining(x,y,w,h);

        // update traits
        _traits->screenNum = screenNdx;

        // get top left of screen
        wsi->getScreenTopLeft((*_traits), screenLeft, screenTop);
    }

    resized(x-screenLeft,y-screenTop,w,h);
}



void GraphicsWindowCarbon::grabFocus()
{
    SelectWindow(_window);
}



void GraphicsWindowCarbon::grabFocusIfPointerInWindow()
{
   // TODO: implement
   OSG_NOTIFY(osg::ALWAYS) << "GraphicsWindowCarbon::grabFocusIfPointerInWindow: not implemented" << std::endl;
}


void GraphicsWindowCarbon::useCursor(bool cursorOn)
{
    if (_traits.valid())
        _traits->useCursor = cursorOn;
    DarwinWindowingSystemInterface* wsi = dynamic_cast<DarwinWindowingSystemInterface*>(osg::GraphicsContext::getWindowingSystemInterface());
    if (wsi == NULL) {
        OSG_WARN << "GraphicsWindowCarbon::useCursor: could not get OSXCarbonWindowingSystemInterface" << std::endl;
        return;
    }

    CGDirectDisplayID displayId = wsi->getDisplayID((*_traits));
    CGDisplayErr err = (cursorOn ? CGDisplayShowCursor(displayId) : CGDisplayHideCursor(displayId));
    if (err != kCGErrorSuccess) {
        OSG_WARN << "GraphicsWindowCarbon::useCursor: failed with " << err << std::endl;
    }
}

// FIXME: need to implement all cursor types
// FIXME: I used deprecated functions, but don't know if there are any substitutable newer functions...
void GraphicsWindowCarbon::setCursor(MouseCursor mouseCursor)
{
    if (_currentCursor == mouseCursor)
      return;

    UInt32 cursor;
    switch (mouseCursor)
    {
        case NoCursor:
          HideCursor();
          _currentCursor = mouseCursor;
          return;
        case RightArrowCursor:
            cursor = kThemeArrowCursor;
            break;
        case CrosshairCursor:
            cursor = kThemeCrossCursor;
            break;
        case TextCursor:
            cursor = kThemeIBeamCursor;
            break;
        case UpDownCursor:
            cursor = kThemeResizeUpDownCursor;
            break;
        case LeftRightCursor:
            cursor = kThemeResizeLeftRightCursor;
            break;
        default:
            cursor = kThemeArrowCursor;
            OSG_WARN << "GraphicsWindowCarbon::setCursor doesn't implement cursor: type = " << mouseCursor << std::endl;
    }

    _currentCursor = mouseCursor;
    SetThemeCursor(cursor);
    ShowCursor();
}

void GraphicsWindowCarbon::setSyncToVBlank(bool on)
{
    if (_traits.valid()) {
        _traits->vsync = on;
    }
}

void GraphicsWindowCarbon::setWindowName (const std::string& name)
{
    _traits->windowName = name;
    if (!_traits->windowName.empty())
    {
        CFStringRef windowtitle = CFStringCreateWithBytes( kCFAllocatorDefault, (const UInt8*)(_traits->windowName.c_str()), _traits->windowName.length(),kCFStringEncodingUTF8, false );
        SetWindowTitleWithCFString( _window, windowtitle );
        CFRelease(windowtitle);
    }
}

void GraphicsWindowCarbon::requestWarpPointer(float x,float y)
{
    if (!_realized)
    {
        OSG_INFO<<"GraphicsWindowCarbon::requestWarpPointer() - Window not realized; cannot warp pointer, screenNum="<< _traits->screenNum<<std::endl;
        return;
    }

    DarwinWindowingSystemInterface* wsi = dynamic_cast<DarwinWindowingSystemInterface*>(osg::GraphicsContext::getWindowingSystemInterface());
    if (wsi == NULL)
    {
        OSG_WARN << "GraphicsWindowCarbon::useCursor: could not get OSXCarbonWindowingSystemInterface" << std::endl;
        return;
    }

    CGDirectDisplayID displayId = wsi->getDisplayID((*_traits));

    CGPoint point;
    point.x = x + _traits->x;
    point.y = y + _traits->y;
    CGDisplayMoveCursorToPoint(displayId, point);

    getEventQueue()->mouseWarped(x,y);
}


void GraphicsWindowCarbon::transformMouseXY(float& x, float& y)
{
    if (getEventQueue()->getUseFixedMouseInputRange())
    {
        osgGA::GUIEventAdapter* eventState = getEventQueue()->getCurrentEventState();
        x = eventState->getXmin() + (eventState->getXmax()-eventState->getXmin())*x/float(_traits->width);
        y = eventState->getYmin() + (eventState->getYmax()-eventState->getYmin())*y/float(_traits->height);
    }
}

class CarbonWindowingSystemInterface : public  DarwinWindowingSystemInterface {
public:
    CarbonWindowingSystemInterface() : DarwinWindowingSystemInterface()
    {
    }

    virtual osg::GraphicsContext* createGraphicsContext(osg::GraphicsContext::Traits* traits)
    {
        _init();

        return createGraphicsContextImplementation<PixelBufferCarbon, GraphicsWindowCarbon>(traits);
    }

    virtual void _init()
    {
        if (_initialized) return;

        DarwinWindowingSystemInterface::_init();

        // register application event handler and AppleEventHandler to get quit-events:
        static const EventTypeSpec menueventSpec = {kEventClassCommand, kEventCommandProcess};
        OSErr status = InstallEventHandler(GetApplicationEventTarget(), NewEventHandlerUPP(ApplicationEventHandler), 1, &menueventSpec, 0, NULL);
        status = AEInstallEventHandler( kCoreEventClass, kAEQuitApplication, NewAEEventHandlerUPP(QuitAppleEventHandler), 0, false);
    }

};


}

#ifdef USE_DARWIN_CARBON_IMPLEMENTATION
RegisterWindowingSystemInterfaceProxy<CarbonWindowingSystemInterface> createWindowingSystemInterfaceProxy;
#endif


// declare C entry point for static compilation.
extern "C" void graphicswindow_Carbon(void)
{
    osg::GraphicsContext::setWindowingSystemInterface(new osgViewer::CarbonWindowingSystemInterface());
}

#endif
