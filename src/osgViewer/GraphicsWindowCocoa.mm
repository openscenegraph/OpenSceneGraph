/*
 *  GraphicsWindowCocoa.cpp
 *  OpenSceneGraph
 *
 *  Created by Stephan Huber on 27.06.08.
 *  Copyright 2008 Stephan Maximilian Huber, digital mind. All rights reserved.
 * 
 *  Some code borrowed from the implementation of CocoaViewer, 
 *  Created by Eric Wing on 11/12/06. and ported by Martin Lavery 7/06/07
 */

#include <iostream>
#include <osgViewer/api/Cocoa/PixelBufferCocoa>
#include <osgViewer/api/Cocoa/GraphicsWindowCocoa>

#include <Cocoa/Cocoa.h>

#include "DarwinUtils.h"


static bool s_quit_requested = false;


// ----------------------------------------------------------------------------------------------------------
// small helper class remapping key-codes
// ----------------------------------------------------------------------------------------------------------
// small helper class which maps the raw key codes to osgGA::GUIEventAdapter::Keys

class CocoaKeyboardMap {

    public:
        CocoaKeyboardMap()
        {
            _keymap[27]     = osgGA::GUIEventAdapter::KEY_Escape;
            _keymap[13]     = osgGA::GUIEventAdapter::KEY_KP_Enter;
            _keymap[3]      = osgGA::GUIEventAdapter::KEY_Return;
            _keymap[9]      = osgGA::GUIEventAdapter::KEY_Tab;
            _keymap[32]     = osgGA::GUIEventAdapter::KEY_Space;
            _keymap[127]    = osgGA::GUIEventAdapter::KEY_BackSpace;
            
            
            _keymap[NSHomeFunctionKey]          = osgGA::GUIEventAdapter::KEY_Home;
            _keymap[NSEndFunctionKey]           = osgGA::GUIEventAdapter::KEY_End;
            _keymap[NSPageUpFunctionKey]        = osgGA::GUIEventAdapter::KEY_Page_Up;
            _keymap[NSPageDownFunctionKey]      = osgGA::GUIEventAdapter::KEY_Page_Down;
            _keymap[NSLeftArrowFunctionKey]     = osgGA::GUIEventAdapter::KEY_Left;
            _keymap[NSRightArrowFunctionKey]    = osgGA::GUIEventAdapter::KEY_Right;
            _keymap[NSUpArrowFunctionKey]       = osgGA::GUIEventAdapter::KEY_Up;
            _keymap[NSDownArrowFunctionKey]     = osgGA::GUIEventAdapter::KEY_Down;
            
            _keymap[NSDeleteFunctionKey]        = osgGA::GUIEventAdapter::KEY_Delete;
            
            _keymap[NSF1FunctionKey]  = osgGA::GUIEventAdapter::KEY_F1;
            _keymap[NSF2FunctionKey]  = osgGA::GUIEventAdapter::KEY_F2;
            _keymap[NSF3FunctionKey]  = osgGA::GUIEventAdapter::KEY_F3;
            _keymap[NSF4FunctionKey]  = osgGA::GUIEventAdapter::KEY_F4;
            _keymap[NSF5FunctionKey]  = osgGA::GUIEventAdapter::KEY_F5;
            _keymap[NSF6FunctionKey]  = osgGA::GUIEventAdapter::KEY_F6;
            _keymap[NSF7FunctionKey]  = osgGA::GUIEventAdapter::KEY_F7;
            _keymap[NSF8FunctionKey]  = osgGA::GUIEventAdapter::KEY_F8;
            _keymap[NSF9FunctionKey]  = osgGA::GUIEventAdapter::KEY_F9;
            
            _keymap[NSF10FunctionKey]  = osgGA::GUIEventAdapter::KEY_F10;
            _keymap[NSF11FunctionKey]  = osgGA::GUIEventAdapter::KEY_F11;
            _keymap[NSF12FunctionKey]  = osgGA::GUIEventAdapter::KEY_F12;
            _keymap[NSF13FunctionKey]  = osgGA::GUIEventAdapter::KEY_F13;
            _keymap[NSF14FunctionKey]  = osgGA::GUIEventAdapter::KEY_F14;
            _keymap[NSF15FunctionKey]  = osgGA::GUIEventAdapter::KEY_F15;
            _keymap[NSF16FunctionKey]  = osgGA::GUIEventAdapter::KEY_F16;
            _keymap[NSF17FunctionKey]  = osgGA::GUIEventAdapter::KEY_F17;
            _keymap[NSF18FunctionKey]  = osgGA::GUIEventAdapter::KEY_F18;
            _keymap[NSF19FunctionKey]  = osgGA::GUIEventAdapter::KEY_F19;
           
            _keymap[NSF20FunctionKey]  = osgGA::GUIEventAdapter::KEY_F20;
            _keymap[NSF21FunctionKey]  = osgGA::GUIEventAdapter::KEY_F21;
            _keymap[NSF22FunctionKey]  = osgGA::GUIEventAdapter::KEY_F22;
            _keymap[NSF23FunctionKey]  = osgGA::GUIEventAdapter::KEY_F23;
            _keymap[NSF24FunctionKey]  = osgGA::GUIEventAdapter::KEY_F24;
            _keymap[NSF25FunctionKey]  = osgGA::GUIEventAdapter::KEY_F25;
            _keymap[NSF26FunctionKey]  = osgGA::GUIEventAdapter::KEY_F26;
            _keymap[NSF27FunctionKey]  = osgGA::GUIEventAdapter::KEY_F27;
            _keymap[NSF28FunctionKey]  = osgGA::GUIEventAdapter::KEY_F28;
            _keymap[NSF29FunctionKey]  = osgGA::GUIEventAdapter::KEY_F29;
            
            _keymap[NSF30FunctionKey]  = osgGA::GUIEventAdapter::KEY_F30;
            _keymap[NSF31FunctionKey]  = osgGA::GUIEventAdapter::KEY_F31;
            _keymap[NSF32FunctionKey]  = osgGA::GUIEventAdapter::KEY_F32;
            _keymap[NSF33FunctionKey]  = osgGA::GUIEventAdapter::KEY_F33;
            _keymap[NSF34FunctionKey]  = osgGA::GUIEventAdapter::KEY_F34;
            _keymap[NSF35FunctionKey]  = osgGA::GUIEventAdapter::KEY_F35;
                        
            
            _keypadmap['='] = osgGA::GUIEventAdapter::KEY_KP_Equal;
            _keypadmap['*'] = osgGA::GUIEventAdapter::KEY_KP_Multiply;
            _keypadmap['+'] = osgGA::GUIEventAdapter::KEY_KP_Add;
            _keypadmap['-'] = osgGA::GUIEventAdapter::KEY_KP_Subtract;
            _keypadmap['.'] = osgGA::GUIEventAdapter::KEY_KP_Decimal;
            _keypadmap['/'] = osgGA::GUIEventAdapter::KEY_KP_Divide;
            
            _keypadmap['0'] = osgGA::GUIEventAdapter::KEY_KP_0;
            _keypadmap['1'] = osgGA::GUIEventAdapter::KEY_KP_1;
            _keypadmap['2'] = osgGA::GUIEventAdapter::KEY_KP_2;
            _keypadmap['3'] = osgGA::GUIEventAdapter::KEY_KP_3;
            _keypadmap['4'] = osgGA::GUIEventAdapter::KEY_KP_4;
            _keypadmap['5'] = osgGA::GUIEventAdapter::KEY_KP_5;
            _keypadmap['6'] = osgGA::GUIEventAdapter::KEY_KP_6;
            _keypadmap['7'] = osgGA::GUIEventAdapter::KEY_KP_7;
            _keypadmap['8'] = osgGA::GUIEventAdapter::KEY_KP_8;
            _keypadmap['9'] = osgGA::GUIEventAdapter::KEY_KP_9;
        }
        
        ~CocoaKeyboardMap() {
        }
        
        unsigned int remapKey(unsigned int key, bool pressedOnKeypad = false)
        {
            if (pressedOnKeypad) {
                 KeyMap::iterator itr = _keypadmap.find(key);
                if (itr == _keypadmap.end()) return key;
                else return itr->second;
            }
            
            KeyMap::iterator itr = _keymap.find(key);
            if (itr == _keymap.end()) return key;
            else return itr->second;
        }
    private:
        typedef std::map<unsigned int, osgGA::GUIEventAdapter::KeySymbol> KeyMap;
        KeyMap _keymap, _keypadmap;
};


/** remaps a native os x keycode to a GUIEventAdapter-keycode */
static unsigned int remapCocoaKey(unsigned int key, bool pressedOnKeypad = false)
{
    static CocoaKeyboardMap s_CocoaKeyboardMap;
    return s_CocoaKeyboardMap.remapKey(key, pressedOnKeypad);
}


// ----------------------------------------------------------------------------------------------------------
// the app-delegate, handling quit-requests
// ----------------------------------------------------------------------------------------------------------

@interface CocoaAppDelegate : NSObject 
{
}

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender;
@end

@implementation CocoaAppDelegate
- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender
{
    s_quit_requested = true;
    std::cout << "quit requested " << std::endl;
    return NSTerminateNow;
}

@end



// ----------------------------------------------------------------------------------------------------------
// GraphicsWindowCocoaWindow
// ----------------------------------------------------------------------------------------------------------

@interface GraphicsWindowCocoaWindow : NSWindow
{
}

- (BOOL) canBecomeKeyWindow;
- (BOOL) canBecomeMainWindow;

@end

@implementation GraphicsWindowCocoaWindow


- (BOOL) canBecomeKeyWindow
{
    return YES;
}

- (BOOL) canBecomeMainWindow
{
    return YES;
}

@end

// ----------------------------------------------------------------------------------------------------------
// GraphicsWindowCocoaGLView
// ----------------------------------------------------------------------------------------------------------


@interface GraphicsWindowCocoaGLView : NSOpenGLView
{
    @private
        osgViewer::GraphicsWindowCocoa* _win;
        BOOL _isUsingCtrlClick, _isUsingOptionClick;
        unsigned int _cachedModifierFlags;
        
}
- (void)setGraphicsWindowCocoa: (osgViewer::GraphicsWindowCocoa*) win;

- (void)keyDown:(NSEvent *)theEvent;
- (void)keyUp:(NSEvent *)theEvent;

- (void) mouseMoved:(NSEvent*)the_event;
- (void) mouseDown:(NSEvent*)the_event;
- (void) mouseDragged:(NSEvent*)the_event;
- (void) mouseUp:(NSEvent*)the_event;
- (void) rightMouseDown:(NSEvent*)the_event;
- (void) rightMouseDragged:(NSEvent*)the_event;
- (void) rightMouseUp:(NSEvent*)the_event;
- (void) otherMouseDown:(NSEvent*)the_event;
- (void) otherMouseDragged:(NSEvent*)the_event;
- (void) otherMouseUp:(NSEvent*)the_event;

- (NSPoint) getLocalPoint: (NSEvent*)the_event;
- (void) handleModifiers: (NSEvent*)the_event;
- (void) setIsUsingCtrlClick:(BOOL)is_using_ctrl_click;
- (BOOL) isUsingCtrlClick;
- (void) setIsUsingOptionClick:(BOOL)is_using_option_click;
- (BOOL) isUsingOptionClick;

- (void) doLeftMouseButtonDown:(NSEvent*)the_event;
- (void) doLeftMouseButtonUp:(NSEvent*)the_event;
- (void) doRightMouseButtonDown:(NSEvent*)the_event;
- (void) doRightMouseButtonUp:(NSEvent*)the_event;
- (void) doMiddleMouseButtonDown:(NSEvent*)the_event;
- (void) doExtraMouseButtonDown:(NSEvent*)the_event buttonNumber:(int)button_number;
- (void) doMiddleMouseButtonUp:(NSEvent*)the_event;
- (void) doExtraMouseButtonUp:(NSEvent*)the_event buttonNumber:(int)button_number;
- (void) scrollWheel:(NSEvent*)the_event;

- (BOOL)acceptsFirstResponder;
- (BOOL)becomeFirstResponder;
- (BOOL)resignFirstResponder;

@end

@implementation GraphicsWindowCocoaGLView 


-(void) setGraphicsWindowCocoa: (osgViewer::GraphicsWindowCocoa*) win
{
    _win = win;
}

- (BOOL)acceptsFirstResponder
{
  return YES;
}

- (BOOL)becomeFirstResponder
{
  return YES;
}

- (BOOL)resignFirstResponder
{
  return YES;
}


- (NSPoint) getLocalPoint: (NSEvent*)the_event
{
    return  [self convertPoint:[the_event locationInWindow] fromView:nil];
}


- (void) handleModifiers: (NSEvent*)the_event
{
    unsigned int flags = [the_event modifierFlags];
    
    if (flags == _cachedModifierFlags) 
        return;
    
    const unsigned int masks[] = { 
        NSShiftKeyMask,
        NSControlKeyMask,
        NSAlternateKeyMask,
        NSCommandKeyMask,
        NSAlphaShiftKeyMask
    };
    
    const unsigned int keys[] = { 
        osgGA::GUIEventAdapter::KEY_Shift_L, 
        osgGA::GUIEventAdapter::KEY_Control_L,
        osgGA::GUIEventAdapter::KEY_Alt_L,
        osgGA::GUIEventAdapter::KEY_Super_L,
        osgGA::GUIEventAdapter::KEY_Caps_Lock
    };
    
    for(unsigned int i = 0; i < 5; ++i) {
        
        if ((flags & masks[i]) && !(_cachedModifierFlags & masks[i]))
        {
            _win->getEventQueue()->keyPress(keys[i]);
        }
        
        if (!(flags & masks[i]) && (_cachedModifierFlags & masks[i]))
        {
            _win->getEventQueue()->keyRelease(keys[i]);
        }
    }
    
    _cachedModifierFlags = flags;
    
}

- (void) mouseMoved:(NSEvent*)the_event 
{
    [self handleModifiers: the_event];
    NSPoint converted_point = [self getLocalPoint: the_event];
    _win->getEventQueue()->mouseMotion(converted_point.x, converted_point.y);
}



- (void) mouseDown:(NSEvent*)the_event
{
    [self handleModifiers: the_event];
    // Because many Mac users have only a 1-button mouse, we should provide ways
    // to access the button 2 and 3 actions of osgViewer.
    // I will use the Ctrl modifer to represent right-clicking
    // and Option modifier to represent middle clicking.
    if([the_event modifierFlags] & NSControlKeyMask)
    {
        [self setIsUsingCtrlClick:YES];
        [self doRightMouseButtonDown:the_event];
    }
    else if([the_event modifierFlags] & NSAlternateKeyMask)
    {
        [self setIsUsingOptionClick:YES];
        [self doMiddleMouseButtonDown:the_event];
    }
    else
    {
        [self doLeftMouseButtonDown:the_event];
    }
}


- (void) mouseDragged:(NSEvent*)the_event
{
    [self handleModifiers: the_event];
    NSPoint converted_point = [self getLocalPoint: the_event];    
    _win->getEventQueue()->mouseMotion(converted_point.x, converted_point.y);
}


- (void) mouseUp:(NSEvent*)the_event
{
    [self handleModifiers: the_event];
    
    // Because many Mac users have only a 1-button mouse, we should provide ways
    // to access the button 2 and 3 actions of osgViewer.
    // I will use the Ctrl modifer to represent right-clicking
    // and Option modifier to represent middle clicking.
    if([self isUsingCtrlClick] == YES)
    {
        [self setIsUsingCtrlClick:NO];
        [self doRightMouseButtonUp:the_event];
    }
    else if([self isUsingOptionClick] == YES)
    {
        [self setIsUsingOptionClick:NO];
        [self doMiddleMouseButtonUp:the_event];
    }
    else
    {
        [self doLeftMouseButtonUp:the_event];
    }
}

- (void) rightMouseDown:(NSEvent*)the_event
{
    [self handleModifiers: the_event];
    [self doRightMouseButtonDown:the_event];
}

- (void) rightMouseDragged:(NSEvent*)the_event
{
    [self handleModifiers: the_event];
    
    NSPoint converted_point = [self getLocalPoint: the_event];
    _win->getEventQueue()->mouseMotion(converted_point.x, converted_point.y);
    
}

- (void) rightMouseUp:(NSEvent*)the_event
{
    [self handleModifiers: the_event];
    [self doRightMouseButtonUp:the_event];
}

// "otherMouse" seems to capture middle button and any other buttons beyond (4th, etc).
- (void) otherMouseDown:(NSEvent*)the_event
{
    [self handleModifiers: the_event];
    // Button 0 is left
    // Button 1 is right
    // Button 2 is middle
    // Button 3 keeps going
    // osgViewer expects 1 for left, 3 for right, 2 for middle
    // osgViewer has a reversed number mapping for right and middle compared to Cocoa
    if([the_event buttonNumber] == 2)
    {
        [self doMiddleMouseButtonDown:the_event];
    }
    else // buttonNumber should be 3,4,5,etc; must map to 4,5,6,etc in osgViewer
    {
        [self doExtraMouseButtonDown:the_event buttonNumber:[the_event buttonNumber]];
    }
}

- (void) otherMouseDragged:(NSEvent*)the_event
{
    [self handleModifiers: the_event];
    NSPoint converted_point = [self getLocalPoint: the_event];    
    _win->getEventQueue()->mouseMotion(converted_point.x, converted_point.y);
   
}

// "otherMouse" seems to capture middle button and any other buttons beyond (4th, etc).
- (void) otherMouseUp:(NSEvent*)the_event
{
    [self handleModifiers: the_event];
    
    // Button 0 is left
    // Button 1 is right
    // Button 2 is middle
    // Button 3 keeps going
    // osgViewer expects 1 for left, 3 for right, 2 for middle
    // osgViewer has a reversed number mapping for right and middle compared to Cocoa
    if([the_event buttonNumber] == 2)
    {
        [self doMiddleMouseButtonUp:the_event];
    }
    else // buttonNumber should be 3,4,5,etc; must map to 4,5,6,etc in osgViewer
    {
        // I don't think osgViewer does anything for these additional buttons,
        // but just in case, pass them along. But as a Cocoa programmer, you might 
        // think about things you can do natively here instead of passing the buck.
        [self doExtraMouseButtonUp:the_event buttonNumber:[the_event buttonNumber]];
    }
}

- (void) setIsUsingCtrlClick:(BOOL)is_using_ctrl_click
{
    _isUsingCtrlClick = is_using_ctrl_click;
}

- (BOOL) isUsingCtrlClick
{
    return _isUsingCtrlClick;
}

- (void) setIsUsingOptionClick:(BOOL)is_using_option_click
{
    _isUsingOptionClick = is_using_option_click;
}

- (BOOL) isUsingOptionClick
{
    return _isUsingOptionClick;
}


- (void) doLeftMouseButtonDown:(NSEvent*)the_event
{
    NSPoint converted_point = [self getLocalPoint: the_event];
    
    if([the_event clickCount] == 1)
    {
        _win->getEventQueue()->mouseButtonPress(converted_point.x, converted_point.y, 1);
    }
    else
    {
        _win->getEventQueue()->mouseDoubleButtonPress(converted_point.x, converted_point.y, 1);
    }
}

- (void) doLeftMouseButtonUp:(NSEvent*)the_event
{
    NSPoint converted_point = [self getLocalPoint: the_event];
    
    _win->getEventQueue()->mouseButtonRelease(converted_point.x, converted_point.y, 1);

}

- (void) doRightMouseButtonDown:(NSEvent*)the_event
{
    NSPoint converted_point = [self getLocalPoint: the_event];
    if([the_event clickCount] == 1)
    {
        _win->getEventQueue()->mouseButtonPress(converted_point.x, converted_point.y, 3);
    }
    else
    {
        _win->getEventQueue()->mouseDoubleButtonPress(converted_point.x, converted_point.y, 3);
    }

}


- (void) doRightMouseButtonUp:(NSEvent*)the_event
{
    NSPoint converted_point = [self getLocalPoint: the_event];    
    _win->getEventQueue()->mouseButtonRelease(converted_point.x, converted_point.y, 3);
}

- (void) doMiddleMouseButtonDown:(NSEvent*)the_event
{
    NSPoint converted_point = [self getLocalPoint: the_event];
    
    if([the_event clickCount] == 1)
    {
        _win->getEventQueue()->mouseButtonPress(converted_point.x, converted_point.y, 2);
    }
    else
    {
        _win->getEventQueue()->mouseDoubleButtonPress(converted_point.x, converted_point.y, 2);
    }
}

- (void) doExtraMouseButtonDown:(NSEvent*)the_event buttonNumber:(int)button_number
{
    NSPoint converted_point = [self getLocalPoint: the_event];
    if([the_event clickCount] == 1)
    {
        _win->getEventQueue()->mouseButtonPress(converted_point.x, converted_point.y, button_number+1);
    }
    else
    {
        _win->getEventQueue()->mouseDoubleButtonPress(converted_point.x, converted_point.y, button_number+1);
    }
}


- (void) doMiddleMouseButtonUp:(NSEvent*)the_event
{
    NSPoint converted_point = [self getLocalPoint: the_event];
    _win->getEventQueue()->mouseButtonRelease(converted_point.x, converted_point.y, 2);

}

- (void) doExtraMouseButtonUp:(NSEvent*)the_event buttonNumber:(int)button_number
{
    NSPoint converted_point = [self getLocalPoint: the_event];
    _win->getEventQueue()->mouseButtonRelease(converted_point.x, converted_point.y, button_number+1);
}



- (void) scrollWheel:(NSEvent*)the_event
{
    // Unfortunately, it turns out mouseScroll2D doesn't actually do anything.
    // The camera manipulators don't seem to implement any code that utilize the scroll values.
    // This this call does nothing.
    _win->getEventQueue()->mouseScroll2D([the_event deltaX], [the_event deltaY]);
}



- (void)keyDown:(NSEvent *)theEvent 
{
    [self handleModifiers: theEvent];
    NSString* chars = [theEvent charactersIgnoringModifiers]; 
    unsigned int keyCode = remapCocoaKey([chars characterAtIndex:0], ([theEvent modifierFlags] & NSFunctionKeyMask) );
    // std::cout << "key dn: " <<[chars characterAtIndex:0] << "=" << keyCode << std::endl;   
    _win->getEventQueue()->keyPress( remapCocoaKey(keyCode), [theEvent timestamp]);
}


- (void)keyUp:(NSEvent *)theEvent 
{   
    [self handleModifiers: theEvent];
    NSString* chars = [theEvent charactersIgnoringModifiers]; 
    unsigned int keyCode = remapCocoaKey([chars characterAtIndex:0], ([theEvent modifierFlags] & NSFunctionKeyMask));
    // std::cout << "key up: " <<[chars characterAtIndex:0] << "=" << keyCode << std::endl;   
    _win->getEventQueue()->keyRelease( remapCocoaKey(keyCode), [theEvent timestamp]);
}


@end


#pragma mark GraphicsWindowCocoaDelegate


// ----------------------------------------------------------------------------------------------------------
// the window-delegate, handles moving/resizing of the window etc.
// ----------------------------------------------------------------------------------------------------------

@interface GraphicsWindowCocoaDelegate : NSObject
{
    @private
        osgViewer::GraphicsWindowCocoa* _win;
        BOOL                            _inDidMove;
}

- (id)initWith: (osgViewer::GraphicsWindowCocoa*) win;
- (void)windowDidMove:(NSNotification *)notification;
- (BOOL)windowShouldClose:(id)window;

@end

@implementation GraphicsWindowCocoaDelegate

- (id)initWith: (osgViewer::GraphicsWindowCocoa*) win
{
    _inDidMove = false;
    _win = win;
    return [super init];
}

- (void)windowDidMove:(NSNotification *)notification
{
    if (_inDidMove) return;
    _inDidMove = true;
    GraphicsWindowCocoaWindow* nswin = _win->getWindow();
    NSRect bounds = [nswin contentRectForFrameRect: [nswin frame] ];
    std::cout << "windowdidmove: " << bounds.origin.x << " " << bounds.origin.y << " " << bounds.size.width << " " << bounds.size.height << std::endl;
   
    _win->resized(bounds.origin.x, bounds.origin.y, bounds.size.width, bounds.size.height);
    _win->getEventQueue()->windowResize(bounds.origin.x, bounds.origin.y, bounds.size.width, bounds.size.height, _win->getEventQueue()->getTime());
    _win->requestRedraw();
    _inDidMove = false;
}

- (BOOL)windowShouldClose:(id)window 
{
    return _win->requestClose();
}

@end


#pragma mark GraphicsWindowCocoa



using namespace osgDarwin;
namespace osgViewer {


// ----------------------------------------------------------------------------------------------------------
// small adapter class to handle the dock/menubar
// ----------------------------------------------------------------------------------------------------------

class CocoaWindowAdapter : public MenubarController::WindowAdapter {
public:
    CocoaWindowAdapter(GraphicsWindowCocoa* win) : MenubarController::WindowAdapter(), _win(win) {}
    
    virtual bool valid() { return (_win.valid() && _win->valid()); }
	
    virtual void getWindowBounds(CGRect& rect) 
    {
        NSRect nsrect = [_win->getWindow() frame];

        rect.origin.x = nsrect.origin.x;
        rect.origin.y = nsrect.origin.y;
        rect.size.width = nsrect.size.width;
        rect.size.height = nsrect.size.height;
    }
	
    virtual osgViewer::GraphicsWindow* getWindow() {return _win.get(); }
private:
    osg::observer_ptr<GraphicsWindowCocoa> _win;
};


void GraphicsWindowCocoa::init()
{
    if (_initialized) return;

    _closeRequested = false;
    _ownsWindow = false;
    _context = NULL;
    _window = NULL;
    _valid = _initialized = true;
}



void GraphicsWindowCocoa::setupNSWindow(NSWindow* win)
{

    [win setReleasedWhenClosed:NO];
	[win setDisplaysWhenScreenProfileChanges:YES];	
	[win setDelegate: [[GraphicsWindowCocoaDelegate alloc] initWith: this] ];
        
    [win makeKeyAndOrderFront:nil];
    [win setAcceptsMouseMovedEvents: YES];

}



bool GraphicsWindowCocoa::realizeImplementation()
{
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
    
    unsigned int style(NSBorderlessWindowMask);
    
    if (_traits->windowDecoration) {
        style = NSClosableWindowMask | NSMiniaturizableWindowMask | NSTitledWindowMask;
        
        // supportsResize works only with windows with titlebar
        if (_traits->supportsResize) 
            style |= NSResizableWindowMask;
    }
    
    
	NSRect rect = NSMakeRect(_traits->x, _traits->y, _traits->width, _traits->height);
	_window = [[GraphicsWindowCocoaWindow alloc] initWithContentRect: rect styleMask: style backing: NSBackingStoreBuffered defer: NO];
	
    if (!_window) {
        osg::notify(osg::WARN) << "GraphicsWindowCarbon::realizeImplementation :: could not create window" << std::endl;
        return false;
    }
    
	 
	NSOpenGLPixelFormatAttribute attr[32];
    int i = 0;
    
    attr[i++] = NSOpenGLPFADepthSize;
    attr[i++] = static_cast<NSOpenGLPixelFormatAttribute>(_traits->depth);

    if (_traits->doubleBuffer) {
        attr[i++] = NSOpenGLPFADoubleBuffer;
    }
    
    if (_traits->alpha) { 
        attr[i++] = NSOpenGLPFAAlphaSize;
        attr[i++] = static_cast<NSOpenGLPixelFormatAttribute>(_traits->alpha);
    }

    if (_traits->stencil) {
        attr[i++] = NSOpenGLPFAStencilSize;
        attr[i++] = static_cast<NSOpenGLPixelFormatAttribute>(_traits->stencil);
    }
  

    if (_traits->sampleBuffers) {
        attr[i++] = NSOpenGLPFASampleBuffers;
        attr[i++] = static_cast<NSOpenGLPixelFormatAttribute>(_traits->sampleBuffers);
        attr[i++] = NSOpenGLPFASamples;
        attr[i++] = static_cast<NSOpenGLPixelFormatAttribute>(_traits->samples);
    }

    
    attr[i++] = NSOpenGLPFAAccelerated;
    attr[i] = static_cast<NSOpenGLPixelFormatAttribute>(0);
    
    // create the context
    NSOpenGLContext* sharedContext = NULL;
    
    GraphicsWindowCocoa* graphicsWindowCocoa = dynamic_cast<GraphicsWindowCocoa*>(_traits->sharedContext);
    if (graphicsWindowCocoa) 
    {
        sharedContext = graphicsWindowCocoa->getContext();
    }
    else
    {
        PixelBufferCocoa* pixelbuffer = dynamic_cast<PixelBufferCocoa*>(_traits->sharedContext);
        if (pixelbuffer) {
            sharedContext = pixelbuffer->getContext();
        }
    }
	
	NSOpenGLPixelFormat* pixelformat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attr ];
    _context = [[NSOpenGLContext alloc] initWithFormat: pixelformat shareContext: sharedContext];
    
    if (!_context) {
        osg::notify(osg::WARN) << "GraphicsWindowCarbon::realizeImplementation :: could not create context" << std::endl;
        return false;
    }
	GraphicsWindowCocoaGLView* theView = [[ GraphicsWindowCocoaGLView alloc ] initWithFrame:[ _window frame ] ];
    [theView setAutoresizingMask:  (NSViewWidthSizable | NSViewHeightSizable) ];
    [theView setGraphicsWindowCocoa: this];
    [theView setOpenGLContext:_context];
	[_window setContentView: theView];
	
    setupNSWindow(_window);
    
    [theView release];
    [pool release];
	
    MenubarController::instance()->attachWindow( new CocoaWindowAdapter(this) );
    
    useCursor(_traits->useCursor);
    setWindowName(_traits->windowName);
    setVSync(_traits->vsync);
    
    MenubarController::instance()->update();
    
    // Cocoa's origin is bottom/left:
    getEventQueue()->getCurrentEventState()->setMouseYOrientation(osgGA::GUIEventAdapter::Y_INCREASING_UPWARDS);
    
    _valid = _initialized = _realized = true;
    return _valid;
}






/** Close the graphics context. */
void GraphicsWindowCocoa::closeImplementation()
{
    _valid = false;
    _realized = false;
    
    // there's a possibility that the MenubarController is destructed already, so prevent a crash:
    MenubarController* mbc = MenubarController::instance();
    if (mbc) mbc->detachWindow(this);
    
	[_window close];
    [_window release];
}

/** Make this graphics context current.*/
bool GraphicsWindowCocoa:: makeCurrentImplementation()
{
	[_context makeCurrentContext];
	return true;
}


/** Release the graphics context.*/
bool GraphicsWindowCocoa::releaseContextImplementation()
{
	[NSOpenGLContext clearCurrentContext];
	return true;
}


/** Swap the front and back buffers.*/
void GraphicsWindowCocoa::swapBuffersImplementation()
{
	[_context flushBuffer];
}


/** Check to see if any events have been generated.*/
void GraphicsWindowCocoa::checkEvents()
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    
    while(1)
    {
        /*  NOTE: It may be better to use something like
            NSEventTrackingRunLoopMode since we don't necessarily want all
            timers/sources/observers to run, only those which would
            run while tracking events.  However, it should be noted that
            NSEventTrackingRunLoopMode is in the common set of modes
            so it may not effectively make much of a difference.
         */
        NSEvent *event = [ [NSApplication sharedApplication]
                nextEventMatchingMask:NSAnyEventMask
                untilDate:[NSDate distantPast]
                inMode:NSDefaultRunLoopMode
                dequeue: YES];
        if(!event)
            break;
        [[NSApplication sharedApplication] sendEvent: event];
    }	
    
    if (_closeRequested)
        getEventQueue()->closeWindow();
        
    if (s_quit_requested) {
        getEventQueue()->quitApplication();
        s_quit_requested = false;
    }
        
	[pool release];
}


/** Set the window's position and size.*/
bool GraphicsWindowCocoa::setWindowRectangleImplementation(int x, int y, int width, int height)
{
    std::cout << "GraphicsWindowCocoa :: realizeImplementation not implemented yet " << std::endl;
	
    MenubarController::instance()->update();
    
    return true;
}


/** Set Window decoration.*/
bool GraphicsWindowCocoa::setWindowDecorationImplementation(bool flag)
{
    if (!_realized) return false;
	
    // unfortunately there's no way to change the decoration of a window, so we create an new one and swap the content
    unsigned int style(NSBorderlessWindowMask);
    
    if (flag) {
        style = NSClosableWindowMask | NSMiniaturizableWindowMask | NSTitledWindowMask;
        
        // supportsResize works only with windows with titlebar
        if (_traits->supportsResize) 
            style |= NSResizableWindowMask;
    }
	NSRect rect = [_window contentRectForFrameRect: [_window frame] ];
	GraphicsWindowCocoaWindow* new_win = [[GraphicsWindowCocoaWindow alloc] initWithContentRect: rect styleMask: style backing: NSBackingStoreBuffered defer: NO];
    
    if (new_win) {
        [new_win setContentView: [_window contentView]];
        setupNSWindow(new_win);
        [new_win setTitle: [_window title]];
        [_window close];
        [_window release];

        _window = new_win;
        [_window makeKeyAndOrderFront: nil];
    }

    
	return true;
}


/** Get focus.*/
void GraphicsWindowCocoa::grabFocus()
{
	[_window makeKeyAndOrderFront: nil];
}


/** Get focus on if the pointer is in this window.*/
void GraphicsWindowCocoa::grabFocusIfPointerInWindow()
{
	std::cout << "GraphicsWindowCocoa :: grabFocusIfPointerInWindow not implemented yet " << std::endl;
}


void GraphicsWindowCocoa::resizedImplementation(int x, int y, int width, int height)
{
	GraphicsContext::resizedImplementation(x, y, width, height);
    
    [_window setContentSize: NSMakeSize(width, height)];
    [_context update];
    //[_window setFrameTopLeftPoint: NSMakePoint(x,y)];   
    MenubarController::instance()->update();
    std::cout << "GraphicsWindowCocoa :: resizedImplementation not implemented yet " << std::endl;
}


void GraphicsWindowCocoa::setWindowName (const std::string & name)
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    
    NSString* title = [NSString stringWithCString: name.c_str() encoding: NSUTF8StringEncoding];
	[_window setTitle: title];
	[title release];
    [pool release];
}


void GraphicsWindowCocoa::useCursor(bool cursorOn)
{
	if (_traits.valid())
        _traits->useCursor = cursorOn;
    DarwinWindowingSystemInterface* wsi = dynamic_cast<DarwinWindowingSystemInterface*>(osg::GraphicsContext::getWindowingSystemInterface());
    if (wsi == NULL) {
        osg::notify(osg::WARN) << "GraphicsWindowCarbon::useCursor :: could not get OSXCarbonWindowingSystemInterface" << std::endl;
        return;
    }
    
    CGDirectDisplayID displayId = wsi->getDisplayID((*_traits));
    CGDisplayErr err = kCGErrorSuccess;
    switch (cursorOn)
    {
        case true:
            err = CGDisplayShowCursor(displayId);
            break;
        case false:
            err = CGDisplayHideCursor(displayId);
            break;
    }
    if (err != kCGErrorSuccess) {
        osg::notify(osg::WARN) << "GraphicsWindowCocoa::useCursor failed with " << err << std::endl;
    }
}


void GraphicsWindowCocoa::setCursor(MouseCursor mouseCursor)
{
	std::cout << "GraphicsWindowCocoa :: setCursor not implemented yet " << std::endl;
}

void GraphicsWindowCocoa::setVSync(bool f) 
{
    GLint VBL(f?1:0);
	[_context setValues:&VBL forParameter:NSOpenGLCPSwapInterval];
}


GraphicsWindowCocoa::~GraphicsWindowCocoa() 
{
}





struct CocoaWindowingSystemInterface : public DarwinWindowingSystemInterface {
	
	CocoaWindowingSystemInterface()
	:	DarwinWindowingSystemInterface() 
	{
		localPool = [[NSAutoreleasePool alloc] init];
        [[NSApplication sharedApplication] setDelegate: [[CocoaAppDelegate alloc] init] ];
	}
	
	virtual osg::GraphicsContext* createGraphicsContext(osg::GraphicsContext::Traits* traits) 
	{
		return createGraphicsContextImplementation<PixelBufferCocoa, GraphicsWindowCocoa>(traits);
	}
	
	virtual ~CocoaWindowingSystemInterface() 
	{
		[localPool release];
	}
	
	NSAutoreleasePool *localPool;

};


}

#ifdef USE_DARWIN_COCOA_IMPLEMENTATION
RegisterWindowingSystemInterfaceProxy<osgViewer::CocoaWindowingSystemInterface> createWindowingSystemInterfaceProxy;
#endif

// declare C entry point for static compilation.
extern "C" void graphicswindow_Cocoa(void)
{
    osg::GraphicsContext::setWindowingSystemInterface(new osgViewer::CocoaWindowingSystemInterface());
}
