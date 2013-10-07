/*
 *  GraphicsWindowCocoa.cpp
 *  OpenSceneGraph
 *
 *  Created by Stephan Huber on 27.06.08.
 *  Copyright 2008 Stephan Maximilian Huber, digital mind. All rights reserved.
 *
 *  Some code borrowed from the implementation of CocoaViewer,
 *  Created by Eric Wing on 11/12/06. and ported by Martin Lavery 7/06/07
 *
 *  Other snippets are borrowed from the Cocoa-implementation of the SDL-lib
 */

#include <iostream>
#include <osgViewer/api/Cocoa/PixelBufferCocoa>
#include <osgViewer/api/Cocoa/GraphicsWindowCocoa>

#include <Cocoa/Cocoa.h>

#include "DarwinUtils.h"

//#define DEBUG_OUT(s) std::cout << "GraphicsWindowCocoa :: " << s << std::endl;
#define DEBUG_OUT(s) ;

static bool s_quit_requested = false;

#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4
@interface NSApplication(NSAppleMenu)
- (void)setAppleMenu:(NSMenu *)menu;
@end
#endif


// ----------------------------------------------------------------------------------------------------------
// small helper class remapping key-codes
// ----------------------------------------------------------------------------------------------------------
// small helper class which maps the raw key codes to osgGA::GUIEventAdapter::Keys

class CocoaKeyboardMap {

    public:
        CocoaKeyboardMap()
        {
            _keymap[27]     = osgGA::GUIEventAdapter::KEY_Escape;
            _keymap[13]     = osgGA::GUIEventAdapter::KEY_Return;
            _keymap[3]      = osgGA::GUIEventAdapter::KEY_KP_Enter;
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


// ----------------------------------------------------------------------------------------------------------
// remapCocoaKey
// ----------------------------------------------------------------------------------------------------------
static unsigned int remapCocoaKey(unsigned int key, unsigned int modifiers)
{
    static CocoaKeyboardMap s_CocoaKeyboardMap;

    bool pressedOnKeypad = modifiers & NSNumericPadKeyMask;
    if (modifiers & NSFunctionKeyMask)
        pressedOnKeypad = false;

    //std::cout << std::hex << "remap " << key << " keypad: " << pressedOnKeypad << " modifiers: " << modifiers << std::endl;

    return s_CocoaKeyboardMap.remapKey(key, pressedOnKeypad);
}


std::ostream& operator<<(std::ostream& os, const NSRect& rect)
{
    os << rect.origin.x << "/" << rect.origin.y << " " << rect.size.width << "x" << rect.size.height;
    return os;
}

// ----------------------------------------------------------------------------------------------------------
// Cocoa uses a coordinate system where its origin is in the bottom left corner,
// osg and quartz uses top left for the origin
//
// these 2 methods convets rects between the different coordinate systems
// ----------------------------------------------------------------------------------------------------------

static NSRect convertFromQuartzCoordinates(const NSRect& rect)
{
    NSRect frame = [[[NSScreen screens] objectAtIndex: 0] frame];
    float y = frame.size.height - rect.origin.y - rect.size.height;
    NSRect converted = NSMakeRect(rect.origin.x, y, rect.size.width, rect.size.height);

    // std::cout << "converting from Quartz " << rect << " to " << converted << " using screen rect " << frame << std::endl;

    return converted;
}

static NSRect convertToQuartzCoordinates(const NSRect& rect)
{
    NSRect frame = [[[NSScreen screens] objectAtIndex: 0] frame];

    float y = frame.size.height - (rect.origin.y + rect.size.height);
    NSRect converted = NSMakeRect(rect.origin.x, y, rect.size.width, rect.size.height);

    // std::cout << "converting To Quartz   " << rect << " to " << converted << " using screen rect " << frame << std::endl;

    return converted;
}

#pragma mark CocoaAppDelegate

// ----------------------------------------------------------------------------------------------------------
// the app-delegate, handling quit-requests
// ----------------------------------------------------------------------------------------------------------

@interface CocoaAppDelegate : NSObject
{
}

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender;
- (void)applicationDidFinishLaunching:(NSNotification *)aNotification;
@end

@implementation CocoaAppDelegate
- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender
{
    s_quit_requested = true;
    DEBUG_OUT("quit requested ");
    return NSTerminateNow;
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    DEBUG_OUT("applicationDidFinishLaunching");
}

@end

#pragma mark GraphicsWindowCocoaWindow

// ----------------------------------------------------------------------------------------------------------
// GraphicsWindowCocoaWindow, implements canBecomeKeyWindow + canBecomeMainWindow
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

#pragma mark GraphicsWindowCocoaGLView


// ----------------------------------------------------------------------------------------------------------
// GraphicsWindowCocoaGLView
// custom NSOpenGLView-class handling mouse- and keyboard-events, forwarding them to the EventQueue
// some code borrowed from the example osgCocoaViewer from E.Wing
// ----------------------------------------------------------------------------------------------------------

@interface GraphicsWindowCocoaGLView : NSOpenGLView
{
    @private
        osgViewer::GraphicsWindowCocoa* _win;
        BOOL _isUsingCtrlClick, _isUsingOptionClick;
        unsigned int _cachedModifierFlags;
        BOOL _handleTabletEvents;

        NSMutableDictionary* _touchPoints;
        unsigned int _lastTouchPointId;
        
}
- (void)setGraphicsWindowCocoa: (osgViewer::GraphicsWindowCocoa*) win;

- (void)keyDown:(NSEvent *)theEvent;
- (void)keyUp:(NSEvent *)theEvent;
- (void)flagsChanged:(NSEvent *)theEvent;
- (void) mouseMoved:(NSEvent*)theEvent;
- (void) mouseDown:(NSEvent*)theEvent;
- (void) mouseDragged:(NSEvent*)theEvent;
- (void) mouseUp:(NSEvent*)theEvent;
- (void) rightMouseDown:(NSEvent*)theEvent;
- (void) rightMouseDragged:(NSEvent*)theEvent;
- (void) rightMouseUp:(NSEvent*)theEvent;
- (void) otherMouseDown:(NSEvent*)theEvent;
- (void) otherMouseDragged:(NSEvent*)theEvent;
- (void) otherMouseUp:(NSEvent*)theEvent;

- (NSPoint) getLocalPoint: (NSEvent*)theEvent;
- (void) handleModifiers: (NSEvent*)theEvent;
- (void) setIsUsingCtrlClick:(BOOL)is_using_ctrl_click;
- (BOOL) isUsingCtrlClick;
- (void) setIsUsingOptionClick:(BOOL)is_using_option_click;
- (BOOL) isUsingOptionClick;

- (void) doLeftMouseButtonDown:(NSEvent*)theEvent;
- (void) doLeftMouseButtonUp:(NSEvent*)theEvent;
- (void) doRightMouseButtonDown:(NSEvent*)theEvent;
- (void) doRightMouseButtonUp:(NSEvent*)theEvent;
- (void) doMiddleMouseButtonDown:(NSEvent*)theEvent;
- (void) doExtraMouseButtonDown:(NSEvent*)theEvent buttonNumber:(int)button_number;
- (void) doMiddleMouseButtonUp:(NSEvent*)theEvent;
- (void) doExtraMouseButtonUp:(NSEvent*)theEvent buttonNumber:(int)button_number;
- (void) scrollWheel:(NSEvent*)theEvent;

- (void)tabletPoint:(NSEvent *)theEvent;
- (void)tabletProximity:(NSEvent *)theEvent;
- (void)handleTabletEvents:(NSEvent*)theEvent;

#if defined(MAC_OS_X_VERSION_10_6) && (MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_6)
- (osgGA::GUIEventAdapter::TouchPhase) convertTouchPhase: (NSTouchPhase) phase;
- (unsigned int)computeTouchId: (NSTouch*) touch mayCleanup: (BOOL) may_cleanup;
- (void)touchesBeganWithEvent:(NSEvent *)event;
- (void)touchesMovedWithEvent:(NSEvent *)event;
- (void)touchesEndedWithEvent:(NSEvent *)event;
- (void)touchesCancelledWithEvent:(NSEvent *)event;
#endif

- (BOOL)useMultiTouchOnly: (NSEvent*) event;

- (BOOL)acceptsFirstResponder;
- (BOOL)becomeFirstResponder;
- (BOOL)resignFirstResponder;

- (void)dealloc;

@end

@implementation GraphicsWindowCocoaGLView


-(void) setGraphicsWindowCocoa: (osgViewer::GraphicsWindowCocoa*) win
{
    _win = win;
    _touchPoints = NULL;
}

-(void) dealloc
{
    if (_touchPoints) [_touchPoints release];
    [super dealloc];
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

- (BOOL) useMultiTouchOnly: (NSEvent*) event
{
#if defined(MAC_OS_X_VERSION_10_6) && (MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_6)
    return ([self acceptsTouchEvents] && ([event subtype] == NSTouchEventSubtype));
#else
    return false;
#endif
}


- (NSPoint) getLocalPoint: (NSEvent*)theEvent
{
    return  [self convertPoint:[theEvent locationInWindow] fromView:nil];
}


- (void) handleModifiers: (NSEvent*)theEvent
{
    DEBUG_OUT("handling modifiers");

    if ((!_win) || (!_win->getEventQueue()))
        return; // no event    queue in place

    unsigned int flags = [theEvent modifierFlags];

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

    // std::cout << "flags: " << flags << " cached: " << _cachedModifierFlags << std::endl;

    for(unsigned int i = 0; i < 5; ++i) {

        if ((flags & masks[i]) && !(_cachedModifierFlags & masks[i]))
        {
            _win->getEventQueue()->keyPress(keys[i], _win->getEventQueue()->getTime(), keys[i]);

            // we don't get a key up for the caps lock so emulate it.
            if (i == 4)
                _win->getEventQueue()->keyRelease(keys[i], _win->getEventQueue()->getTime(), keys[i]);
        }

        if (!(flags & masks[i]) && (_cachedModifierFlags & masks[i]))
        {
            if (i == 4) {
                // emulate a key down for caps-lock.
                _win->getEventQueue()->keyPress(keys[i], _win->getEventQueue()->getTime(), keys[i]);
            }

            _win->getEventQueue()->keyRelease(keys[i], _win->getEventQueue()->getTime(), keys[i]);
        }
    }

    _cachedModifierFlags = flags;

}

- (void)flagsChanged:(NSEvent *)theEvent {
    [self handleModifiers: theEvent];
}

- (void) mouseMoved:(NSEvent*)theEvent
{
    // if multitouch is enabled, disable standard event handling
    if ([self useMultiTouchOnly: theEvent])
        return;
    
    NSPoint converted_point = [self getLocalPoint: theEvent];
    DEBUG_OUT("Mouse moved" << converted_point.x << "/" << converted_point.y);
    _win->getEventQueue()->mouseMotion(converted_point.x, converted_point.y);
}



- (void) mouseDown:(NSEvent*)theEvent
{
    // if multitouch is enabled, disable standard event handling
    if ([self useMultiTouchOnly: theEvent])
        return;
        
    DEBUG_OUT("Mouse down");
    // Because many Mac users have only a 1-button mouse, we should provide ways
    // to access the button 2 and 3 actions of osgViewer.
    // I will use the Ctrl modifer to represent right-clicking
    // and Option modifier to represent middle clicking.
    if([theEvent modifierFlags] & NSControlKeyMask)
    {
        [self setIsUsingCtrlClick:YES];
        [self doRightMouseButtonDown:theEvent];
    }
    else if([theEvent modifierFlags] & NSAlternateKeyMask)
    {
        [self setIsUsingOptionClick:YES];
        [self doMiddleMouseButtonDown:theEvent];
    }
    else
    {
        [self doLeftMouseButtonDown:theEvent];
    }

    if ([theEvent subtype] == NSTabletPointEventSubtype) {
        _handleTabletEvents = true;
        [self handleTabletEvents:theEvent];
    }
}


- (void) mouseDragged:(NSEvent*)theEvent
{
    // if multitouch is enabled, disable standard event handling
    if ([self useMultiTouchOnly: theEvent])
        return;
    
    if (!_win) return;

    NSPoint converted_point = [self getLocalPoint: theEvent];
    _win->getEventQueue()->mouseMotion(converted_point.x, converted_point.y);

    if (_handleTabletEvents)
        [self handleTabletEvents:theEvent];
}


- (void) mouseUp:(NSEvent*)theEvent
{
    // if multitouch is enabled, disable standard event handling
    if ([self useMultiTouchOnly: theEvent])
        return;
        
    // Because many Mac users have only a 1-button mouse, we should provide ways
    // to access the button 2 and 3 actions of osgViewer.
    // I will use the Ctrl modifer to represent right-clicking
    // and Option modifier to represent middle clicking.
    if([self isUsingCtrlClick] == YES)
    {
        [self setIsUsingCtrlClick:NO];
        [self doRightMouseButtonUp:theEvent];
    }
    else if([self isUsingOptionClick] == YES)
    {
        [self setIsUsingOptionClick:NO];
        [self doMiddleMouseButtonUp:theEvent];
    }
    else
    {
        [self doLeftMouseButtonUp:theEvent];
    }
    _handleTabletEvents = false;
}

- (void) rightMouseDown:(NSEvent*)theEvent
{
    [self doRightMouseButtonDown:theEvent];
}

- (void) rightMouseDragged:(NSEvent*)theEvent
{
    if (!_win) return;

    NSPoint converted_point = [self getLocalPoint: theEvent];
    _win->getEventQueue()->mouseMotion(converted_point.x, converted_point.y);
}

- (void) rightMouseUp:(NSEvent*)theEvent
{
    [self doRightMouseButtonUp:theEvent];
    _handleTabletEvents = false;
}

// "otherMouse" seems to capture middle button and any other buttons beyond (4th, etc).
- (void) otherMouseDown:(NSEvent*)theEvent
{
    // Button 0 is left
    // Button 1 is right
    // Button 2 is middle
    // Button 3 keeps going
    // osgViewer expects 1 for left, 3 for right, 2 for middle
    // osgViewer has a reversed number mapping for right and middle compared to Cocoa
    if([theEvent buttonNumber] == 2)
    {
        [self doMiddleMouseButtonDown:theEvent];
    }
    else // buttonNumber should be 3,4,5,etc; must map to 4,5,6,etc in osgViewer
    {
        [self doExtraMouseButtonDown:theEvent buttonNumber:[theEvent buttonNumber]];
    }
}

- (void) otherMouseDragged:(NSEvent*)theEvent
{
    if (!_win) return;

    NSPoint converted_point = [self getLocalPoint: theEvent];
    _win->getEventQueue()->mouseMotion(converted_point.x, converted_point.y);

}

// "otherMouse" seems to capture middle button and any other buttons beyond (4th, etc).
- (void) otherMouseUp:(NSEvent*)theEvent
{

    // Button 0 is left
    // Button 1 is right
    // Button 2 is middle
    // Button 3 keeps going
    // osgViewer expects 1 for left, 3 for right, 2 for middle
    // osgViewer has a reversed number mapping for right and middle compared to Cocoa
    if([theEvent buttonNumber] == 2)
    {
        [self doMiddleMouseButtonUp:theEvent];
    }
    else // buttonNumber should be 3,4,5,etc; must map to 4,5,6,etc in osgViewer
    {
        // I don't think osgViewer does anything for these additional buttons,
        // but just in case, pass them along. But as a Cocoa programmer, you might
        // think about things you can do natively here instead of passing the buck.
        [self doExtraMouseButtonUp:theEvent buttonNumber:[theEvent buttonNumber]];
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


- (void)doSingleOrDoubleButtonPress:(NSEvent*)event forButton:(int)button
{
    if (!_win)
    {
        return;
    }

    NSPoint convertedPoint = [self getLocalPoint:event];
    if ([event clickCount] == 1)
    {
        _win->getEventQueue()->mouseButtonPress(convertedPoint.x, convertedPoint.y, button);
    }
    else
    {
        _win->getEventQueue()->mouseDoubleButtonPress(convertedPoint.x, convertedPoint.y, button);
    }

    //[self setNeedsDisplay:YES];
}

- (void)doButtonRelease:(NSEvent*)event forButton:(int)button
{
    if (!_win)
    {
        return;
    }

    NSPoint convertedPoint = [self getLocalPoint:event];
    _win->getEventQueue()->mouseButtonRelease(convertedPoint.x, convertedPoint.y, button);
    //[self setNeedsDisplay:YES];
}


// Left mouse button
- (void) doLeftMouseButtonDown:(NSEvent*)theEvent
{
    [self doSingleOrDoubleButtonPress:theEvent forButton:1];
}

- (void) doLeftMouseButtonUp:(NSEvent*)theEvent
{
    [self doButtonRelease:theEvent forButton:1];
}

// Right mouse button
- (void) doRightMouseButtonDown:(NSEvent*)theEvent
{
    [self doSingleOrDoubleButtonPress:theEvent forButton:3];
}

- (void) doRightMouseButtonUp:(NSEvent*)theEvent
{
    [self doButtonRelease:theEvent forButton:3];
}

// Middle mouse button
- (void) doMiddleMouseButtonDown:(NSEvent*)theEvent
{
    [self doSingleOrDoubleButtonPress:theEvent forButton:2];
}

- (void) doMiddleMouseButtonUp:(NSEvent*)theEvent
{
    [self doButtonRelease:theEvent forButton:2];
}

// Extra mouse buttons
- (void) doExtraMouseButtonDown:(NSEvent*)theEvent buttonNumber:(int)button_number
{
    [self doButtonRelease:theEvent forButton:(button_number+1)];
}

- (void) doExtraMouseButtonUp:(NSEvent*)theEvent buttonNumber:(int)button_number
{
    [self doButtonRelease:theEvent forButton:(button_number+1)];
}


- (void) scrollWheel:(NSEvent*)theEvent
{
    if (!_win) return;

    // Unfortunately, it turns out mouseScroll2D doesn't actually do anything.
    // The camera manipulators don't seem to implement any code that utilize the scroll values.
    // This this call does nothing.
    _win->getEventQueue()->mouseScroll2D([theEvent deltaX], [theEvent deltaY]);
}



- (void)keyDown:(NSEvent *)theEvent
{
    if (!_win) return;

    NSString* unmodified_chars = [theEvent charactersIgnoringModifiers];
    if ([theEvent modifierFlags] && NSShiftKeyMask) {
        unmodified_chars = [unmodified_chars lowercaseString];
    }

    NSString* chars = [theEvent characters];

    if ((chars) && ([chars length] > 0)) {
        unsigned int unmodified_keyCode = remapCocoaKey([unmodified_chars characterAtIndex:0], [theEvent modifierFlags] );
        unsigned int keyCode = remapCocoaKey([chars characterAtIndex:0], [theEvent modifierFlags] );
        //std::cout << std::hex << "key dn: " <<[chars characterAtIndex:0] << "=" << keyCode << " unmodified: " << unmodified_keyCode <<  std::endl;
        _win->getEventQueue()->keyPress( keyCode, _win->getEventQueue()->getTime(), unmodified_keyCode);
    }
}


- (void)keyUp:(NSEvent *)theEvent
{
    if (!_win) return;

    NSString* unmodified_chars = [theEvent charactersIgnoringModifiers];
    if ([theEvent modifierFlags] && NSShiftKeyMask) {
        unmodified_chars = [unmodified_chars lowercaseString];
    }

    NSString* chars = [theEvent characters];

    if ((chars) && ([chars length] > 0)) {
        unsigned int unmodified_keyCode = remapCocoaKey([unmodified_chars characterAtIndex:0], [theEvent modifierFlags] );
        unsigned int keyCode = remapCocoaKey([chars characterAtIndex:0], [theEvent modifierFlags] );
        //std::cout << std::hex << "key up: " <<[chars characterAtIndex:0] << "=" << keyCode << " unmodified: " << unmodified_keyCode <<  std::endl;

        _win->getEventQueue()->keyRelease( keyCode, _win->getEventQueue()->getTime(), unmodified_keyCode);
    }
}


- (void)tabletPoint:(NSEvent *)theEvent
{
    //_handleTabletEvents = YES;
    //[self handleTabletEvents:theEvent];
}

-(void)handleTabletEvents:(NSEvent *)theEvent
{
    if (!_win) return;

    float pressure = [theEvent pressure];
    _win->getEventQueue()->penPressure(pressure);
    NSPoint tilt = [theEvent tilt];

    _win->getEventQueue()->penOrientation (tilt.x, tilt.y, [theEvent rotation]);
}


- (void)tabletProximity:(NSEvent *)theEvent
{
    if (!_win) return;

    osgGA::GUIEventAdapter::TabletPointerType pt(osgGA::GUIEventAdapter::UNKNOWN);
    switch ([theEvent pointingDeviceType]) {
        case NSPenPointingDevice:
            pt = osgGA::GUIEventAdapter::PEN;
            break;
        case NSCursorPointingDevice:
            pt = osgGA::GUIEventAdapter::PUCK;
            break;
        case NSEraserPointingDevice:
            pt = osgGA::GUIEventAdapter::ERASER;
            break;
        default:
            break;
    }
    _win->getEventQueue()->penProximity(pt, [theEvent isEnteringProximity]);
}

#if defined(MAC_OS_X_VERSION_10_6) && (MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_6)

- (osgGA::GUIEventAdapter::TouchPhase) convertTouchPhase: (NSTouchPhase) phase 
{
    switch(phase) {
    
        case NSTouchPhaseBegan:
            return osgGA::GUIEventAdapter::TOUCH_BEGAN;
            break;
        case NSTouchPhaseMoved:
            return osgGA::GUIEventAdapter::TOUCH_MOVED;
            break;

        case NSTouchPhaseStationary:
            return osgGA::GUIEventAdapter::TOUCH_STATIONERY;
            break;

        case NSTouchPhaseEnded:
        case NSTouchPhaseCancelled:
            return osgGA::GUIEventAdapter::TOUCH_ENDED;
            break;
    }
    
    return osgGA::GUIEventAdapter::TOUCH_ENDED;

}


- (unsigned int)computeTouchId: (NSTouch*) touch mayCleanup: (BOOL) may_cleanup
{
    unsigned int result(0);
    
    if(!_touchPoints) {
        _touchPoints = [[NSMutableDictionary alloc] init];
        _lastTouchPointId = 0;
    }
    
    switch([touch phase])
    {
    
        case NSTouchPhaseBegan:
            if ([_touchPoints objectForKey: [touch identity]] == nil) 
            {
                [_touchPoints setObject: [NSNumber numberWithInt: _lastTouchPointId] forKey: [touch identity]];
                result = _lastTouchPointId++;
                break;
            }
            // missing "break" by intention!
        
        case NSTouchPhaseMoved:
        case NSTouchPhaseStationary:
            {
                NSNumber* n = [_touchPoints objectForKey: [touch identity]];
                result = [n intValue];
            }
            break;
       
        case NSTouchPhaseEnded:
        case NSTouchPhaseCancelled:
            {
                NSNumber* n = [_touchPoints objectForKey: [touch identity]];
                result = [n intValue];
                if(may_cleanup)
                    [_touchPoints removeObjectForKey: [touch identity]];
                if([_touchPoints count] == 0) {
                    _lastTouchPointId = 0;
                }
            }
            break;
            
        default:
            break;
    }
        
    return result;
}



- (void)touchesBeganWithEvent:(NSEvent *)event
{
    NSSet *allTouches = [event touchesMatchingPhase: NSTouchPhaseAny inView: self];
    
    osg::ref_ptr<osgGA::GUIEventAdapter> osg_event(NULL);
    
    NSRect bounds = [self bounds];
    for(unsigned int i=0; i<[allTouches count]; i++)
    {
        
        NSTouch *touch = [[allTouches allObjects] objectAtIndex:i];
        NSPoint pos = [touch normalizedPosition];
        osg::Vec2 pixelPos(pos.x * bounds.size.width, (1-pos.y) * bounds.size.height);
        unsigned int touch_id = [self computeTouchId: touch mayCleanup:FALSE];
        if (!osg_event) {
            osg_event = _win->getEventQueue()->touchBegan(touch_id, [self convertTouchPhase: [touch phase]], pixelPos.x(), pixelPos.y());
        } else {
            osg_event->addTouchPoint(touch_id, [self convertTouchPhase: [touch phase]], pixelPos.x(), pixelPos.y());
        }
    }
}

- (void)touchesMovedWithEvent:(NSEvent *)event
{
    NSSet *allTouches = [event touchesMatchingPhase: NSTouchPhaseAny inView: self];
    
    osg::ref_ptr<osgGA::GUIEventAdapter> osg_event(NULL);
    NSRect bounds = [self bounds];
    
    for(unsigned int i=0; i<[allTouches count]; i++)
    {
        NSTouch *touch = [[allTouches allObjects] objectAtIndex:i];
        NSPoint pos = [touch normalizedPosition];
        osg::Vec2 pixelPos(pos.x * bounds.size.width, (1 - pos.y) * bounds.size.height);
        unsigned int touch_id = [self computeTouchId: touch mayCleanup:FALSE];
        if (!osg_event) {
            osg_event = _win->getEventQueue()->touchMoved(touch_id, [self convertTouchPhase: [touch phase]], pixelPos.x(), pixelPos.y());
        } else {
            osg_event->addTouchPoint(touch_id, [self convertTouchPhase: [touch phase]], pixelPos.x(), pixelPos.y());
        }
    }
}


- (void)touchesEndedWithEvent:(NSEvent *)event
{
    NSSet *allTouches = [event touchesMatchingPhase: NSTouchPhaseAny inView: self];
    
    osg::ref_ptr<osgGA::GUIEventAdapter> osg_event(NULL);
    NSRect bounds = [self bounds];
    
    for(unsigned int i=0; i<[allTouches count]; i++)
    {
        NSTouch *touch = [[allTouches allObjects] objectAtIndex:i];
        NSPoint pos = [touch normalizedPosition];
        osg::Vec2 pixelPos(pos.x * bounds.size.width, (1 - pos.y) * bounds.size.height);
        unsigned int touch_id = [self computeTouchId: touch mayCleanup: TRUE];
        if (!osg_event) {
            osg_event = _win->getEventQueue()->touchEnded(touch_id, [self convertTouchPhase: [touch phase]], pixelPos.x(), pixelPos.y(), 1);
        } else {
            osg_event->addTouchPoint(touch_id, [self convertTouchPhase: [touch phase]], pixelPos.x(), pixelPos.y(), 1);
        }

    }
}



- (void)touchesCancelledWithEvent:(NSEvent *)event
{
    [self touchesEndedWithEvent: event];
}


#endif

@end


#pragma mark GraphicsWindowCocoaDelegate

#ifndef MAC_OS_X_VERSION_10_6
#define MAC_OS_X_VERSION_10_6 1060
#endif



// ----------------------------------------------------------------------------------------------------------
// the window-delegate, handles moving/resizing of the window etc.
// ----------------------------------------------------------------------------------------------------------

@interface GraphicsWindowCocoaDelegate : NSObject
#if MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_6
<NSWindowDelegate>
#endif
{
    @private
        osgViewer::GraphicsWindowCocoa* _win;
        BOOL                            _inDidMove;
}

- (id)initWith: (osgViewer::GraphicsWindowCocoa*) win;
- (void)windowDidMove:(NSNotification *)notification;
- (void)windowDidResize:(NSNotification *)notification;
- (BOOL)windowShouldClose:(id)window;
- (void)updateWindowBounds;

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
    [self updateWindowBounds];
}

- (void)windowDidResize:(NSNotification *)notification
{
    [self updateWindowBounds];
}

-(void)updateWindowBounds
{
    if (_inDidMove) return;
    _inDidMove = true;

    GraphicsWindowCocoaWindow* nswin = _win->getWindow();
    NSRect bounds = [nswin contentRectForFrameRect: [nswin frame] ];

    // convert to quartz-coordinate-system
    bounds = convertToQuartzCoordinates(bounds);

    // std::cout << "windowdidmove: " << bounds.origin.x << " " << bounds.origin.y << " " << bounds.size.width << " " << bounds.size.height << std::endl;

    _win->adaptResize(bounds.origin.x, bounds.origin.y, bounds.size.width, bounds.size.height);
    //_win->getEventQueue()->windowResize(bounds.origin.x, bounds.origin.y, bounds.size.width, bounds.size.height, _win->getEventQueue()->getTime());
    _win->requestRedraw();
    _inDidMove = false;
}

- (BOOL)windowShouldClose:(id)window
{
    return _win->requestClose();
}

@end


#pragma mark CocoaWindowAdapter



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
        nsrect = convertToQuartzCoordinates(nsrect);

        rect.origin.x = nsrect.origin.x;
        rect.origin.y = nsrect.origin.y;
        rect.size.width = nsrect.size.width;
        rect.size.height = nsrect.size.height;
    }

    virtual osgViewer::GraphicsWindow* getWindow() {return _win.get(); }
private:
    osg::observer_ptr<GraphicsWindowCocoa> _win;
};

#pragma mark GraphicsWindowCocoa



// ----------------------------------------------------------------------------------------------------------
// init
// ----------------------------------------------------------------------------------------------------------

void GraphicsWindowCocoa::init()
{
    if (_initialized) return;

    _closeRequested = false;
    _ownsWindow = false;
    _context = NULL;
    _window = NULL;
    _pixelformat = NULL;

    _updateContext = false;
    _valid = _initialized = true;
    
    // make sure the event queue has the correct window rectangle size and input range
    getEventQueue()->syncWindowRectangleWithGraphcisContext();
}


// ----------------------------------------------------------------------------------------------------------
// setupNSWindow
// sets up the NSWindow, adds delegates, etc
// ----------------------------------------------------------------------------------------------------------

void GraphicsWindowCocoa::setupNSWindow(NSWindow* win)
{

    [win setReleasedWhenClosed:NO];
    [win setDisplaysWhenScreenProfileChanges:YES];
    GraphicsWindowCocoaDelegate* delegate = [[GraphicsWindowCocoaDelegate alloc] initWith: this];
    [win setDelegate: delegate ];
    //[delegate autorelease];

    [win makeKeyAndOrderFront:nil];
    [win setAcceptsMouseMovedEvents: YES];

}


// ----------------------------------------------------------------------------------------------------------
// realizeImplementation, creates the window + context
// ----------------------------------------------------------------------------------------------------------

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

    DarwinWindowingSystemInterface* wsi = dynamic_cast<DarwinWindowingSystemInterface*>(osg::GraphicsContext::getWindowingSystemInterface());
    int screenLeft(0), screenTop(0);
    if (wsi) {
        wsi->getScreenTopLeft((*_traits), screenLeft, screenTop);
    }

    NSRect rect = NSMakeRect(_traits->x + screenLeft, _traits->y + screenTop, _traits->width, _traits->height);

    _ownsWindow = true;

    // should we create a NSView only??
    WindowData* windowData = _traits->inheritedWindowData ? dynamic_cast<WindowData*>(_traits->inheritedWindowData.get()) : NULL;
    if (windowData)
    {
        if (windowData->createOnlyView())
            _ownsWindow = false;
        _checkForEvents = windowData->checkForEvents();

    }


    OSG_DEBUG << "GraphicsWindowCocoa::realizeImplementation / ownsWindow: " << _ownsWindow << " checkForEvents: " << _checkForEvents << std::endl;

    if (_ownsWindow)
    {
        _window = [[GraphicsWindowCocoaWindow alloc] initWithContentRect: rect styleMask: style backing: NSBackingStoreBuffered defer: NO];

        if (!_window) {
            OSG_WARN << "GraphicsWindowCocoa::realizeImplementation :: could not create window" << std::endl;
            return false;
        }

        rect = convertFromQuartzCoordinates(rect);
        [_window setFrameOrigin: rect.origin];
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

    GraphicsHandleCocoa* graphicsHandleCocoa = dynamic_cast<GraphicsHandleCocoa*>(_traits->sharedContext.get());
    if (graphicsHandleCocoa)
    {
        sharedContext = graphicsHandleCocoa->getNSOpenGLContext();
    }

    _pixelformat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attr ];
    _context = [[NSOpenGLContext alloc] initWithFormat: _pixelformat shareContext: sharedContext];

    if (!_context) {
        OSG_WARN << "GraphicsWindowCocoa::realizeImplementation :: could not create context" << std::endl;
        return false;
    }

    // set graphics handle for shared usage
    setNSOpenGLContext(_context);

    
    _view = [[ GraphicsWindowCocoaGLView alloc ] initWithFrame:[ _window frame ] ];
    [_view setAutoresizingMask:  (NSViewWidthSizable | NSViewHeightSizable) ];
    [_view setGraphicsWindowCocoa: this];
    [_view setOpenGLContext:_context];
    
    // enable multitouch
    if (_multiTouchEnabled || (windowData && windowData->isMultiTouchEnabled()))
    {
        setMultiTouchEnabled(true);
    }
    
    OSG_DEBUG << "GraphicsWindowCocoa::realizeImplementation / view: " << _view << std::endl;

    if (_ownsWindow) {
        [_window setContentView: _view];
        setupNSWindow(_window);
        [_view release];
        
        MenubarController::instance()->attachWindow( new CocoaWindowAdapter(this) );
    }
    else
    {
        windowData->setCreatedNSView(_view);
    }

    [pool release];


    useCursor(_traits->useCursor);
    setWindowName(_traits->windowName);
    setSyncToVBlank(_traits->vsync);

    MenubarController::instance()->update();

    // Cocoa's origin is bottom/left:
    getEventQueue()->getCurrentEventState()->setMouseYOrientation(osgGA::GUIEventAdapter::Y_INCREASING_UPWARDS);

    // make sure the event queue has the correct window rectangle size and input range
    getEventQueue()->syncWindowRectangleWithGraphcisContext();

    _valid = _initialized = _realized = true;
    return _valid;
}




// ----------------------------------------------------------------------------------------------------------
// closeImplementation
// ----------------------------------------------------------------------------------------------------------
void GraphicsWindowCocoa::closeImplementation()
{
    _valid = false;
    _realized = false;

    // there's a possibility that the MenubarController is destructed already, so prevent a crash:
    MenubarController* mbc = MenubarController::instance();
    if (mbc) mbc->detachWindow(this);

    if (_view) {
        [_view setGraphicsWindowCocoa: NULL];
    }

    if (_window) {
        NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

        // we have to close + release the window in the main-thread

        [_window performSelectorOnMainThread: @selector(close) withObject:NULL waitUntilDone: YES];
        [_window performSelectorOnMainThread: @selector(release) withObject:NULL waitUntilDone: YES];
        [pool release];
    }

    _window = NULL;
    _view = NULL;
}


// ----------------------------------------------------------------------------------------------------------
// makeCurrentImplementation
// ----------------------------------------------------------------------------------------------------------

bool GraphicsWindowCocoa:: makeCurrentImplementation()
{
    if (_updateContext)
    {
        [_context update];
        _updateContext = false;
    }

    [_context makeCurrentContext];
    return true;
}


// ----------------------------------------------------------------------------------------------------------
// releaseContextImplementation
// ----------------------------------------------------------------------------------------------------------

bool GraphicsWindowCocoa::releaseContextImplementation()
{
    [NSOpenGLContext clearCurrentContext];
    return true;
}


// ----------------------------------------------------------------------------------------------------------
// swapBuffersImplementation
// ----------------------------------------------------------------------------------------------------------

void GraphicsWindowCocoa::swapBuffersImplementation()
{
    [_context flushBuffer];
}


// ----------------------------------------------------------------------------------------------------------
// checkEvents
// process all pending events
// ----------------------------------------------------------------------------------------------------------
bool GraphicsWindowCocoa::checkEvents()
{
    if (!_checkForEvents)
        return false;

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
        NSEvent *event = [ NSApp
                nextEventMatchingMask:NSAnyEventMask
                untilDate:[NSDate distantPast]
                inMode:NSDefaultRunLoopMode
                dequeue: YES];
        if(!event)
            break;
        [NSApp sendEvent: event];
    }

    if (_closeRequested)
        getEventQueue()->closeWindow();

    if (s_quit_requested) {
        getEventQueue()->quitApplication();
        s_quit_requested = false;
    }

    [pool release];
    
    return !(getEventQueue()->empty());
}



// ----------------------------------------------------------------------------------------------------------
// setWindowDecorationImplementation
//
// unfortunately there's no way to change the decoration of a window, so we create an new one
// and swap the content
// ----------------------------------------------------------------------------------------------------------

bool GraphicsWindowCocoa::setWindowDecorationImplementation(bool flag)
{
    if (!_realized || !_ownsWindow) return false;

    NSAutoreleasePool* localPool = [[NSAutoreleasePool alloc] init];

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
        NSString* title = (_traits.valid()) ? [NSString stringWithUTF8String: _traits->windowName.c_str()] : @"";
        [new_win setTitle: title ];
        [_window close];
        [_window release];

        _window = new_win;
        [_window makeKeyAndOrderFront: nil];
    }

    [localPool release];

    return true;
}


// ----------------------------------------------------------------------------------------------------------
// grabFocus
// ----------------------------------------------------------------------------------------------------------
void GraphicsWindowCocoa::grabFocus()
{
    if (_ownsWindow)
        [_window makeKeyAndOrderFront: nil];
}


// ----------------------------------------------------------------------------------------------------------
// grabFocusIfPointerInWindow
// ----------------------------------------------------------------------------------------------------------
void GraphicsWindowCocoa::grabFocusIfPointerInWindow()
{
    OSG_INFO << "GraphicsWindowCocoa :: grabFocusIfPointerInWindow not implemented yet " << std::endl;
}


// ----------------------------------------------------------------------------------------------------------
// resizedImplementation
// ----------------------------------------------------------------------------------------------------------

void GraphicsWindowCocoa::resizedImplementation(int x, int y, int width, int height)
{
    DEBUG_OUT("resized implementation" << x << " " << y << " " << width << " " << height);
    GraphicsContext::resizedImplementation(x, y, width, height);

    _updateContext = true;

    MenubarController::instance()->update();
    getEventQueue()->windowResize(x,y,width, height, getEventQueue()->getTime());
}




// ----------------------------------------------------------------------------------------------------------
// setWindowRectangleImplementation
// ----------------------------------------------------------------------------------------------------------
bool GraphicsWindowCocoa::setWindowRectangleImplementation(int x, int y, int width, int height)
{
    if (!_ownsWindow)
        return false;

    NSAutoreleasePool* localPool = [[NSAutoreleasePool alloc] init];

    DarwinWindowingSystemInterface* wsi = dynamic_cast<DarwinWindowingSystemInterface*>(osg::GraphicsContext::getWindowingSystemInterface());
    int screenLeft(0), screenTop(0);
    if (wsi) {
        wsi->getScreenTopLeft((*_traits), screenLeft, screenTop);
    }


    NSRect rect = NSMakeRect(x+screenLeft,y+screenTop,width, height);
    rect = convertFromQuartzCoordinates(rect);

    [_window setFrame: [NSWindow frameRectForContentRect: rect styleMask: [_window styleMask]] display: YES];
    [_context update];
    MenubarController::instance()->update();

    [localPool release];

    return true;
}


// ----------------------------------------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------------------------------------

void GraphicsWindowCocoa::adaptResize(int x, int y, int w, int h)
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
    getEventQueue()->windowResize(x-screenLeft, y-screenTop, w, h, getEventQueue()->getTime());
}


// ----------------------------------------------------------------------------------------------------------
// setWindowName
// ----------------------------------------------------------------------------------------------------------

void GraphicsWindowCocoa::setWindowName (const std::string & name)
{
    if (_traits.valid()) _traits->windowName = name;

    if (!_ownsWindow)
        return;

    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

    NSString* title = [NSString stringWithUTF8String: name.c_str()];
    [_window setTitle: title];
    [pool release];
}

// ----------------------------------------------------------------------------------------------------------
// requestWarpPointer
// ----------------------------------------------------------------------------------------------------------
void GraphicsWindowCocoa::requestWarpPointer(float x,float y)
{    
    CGPoint point;
    point.x = x + _traits->x;
    point.y = y + _traits->y;
    
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4
    CGEventRef warpEvent = CGEventCreateMouseEvent(NULL, kCGEventMouseMoved, point, kCGMouseButtonLeft);
    CGEventPost(kCGHIDEventTap, warpEvent);
    CFRelease(warpEvent);
    
#else
    DarwinWindowingSystemInterface* wsi = dynamic_cast<DarwinWindowingSystemInterface*>(osg::GraphicsContext::getWindowingSystemInterface());
    
    if (wsi == NULL) {
        osg::notify(osg::WARN) << "GraphicsWindowCocoa::useCursor :: could not get OSXCocoaWindowingSystemInterface" << std::endl;
        return;
    }
    
    CGDirectDisplayID displayId = wsi->getDisplayID((*_traits));
    CGSetLocalEventsSuppressionInterval(0);
    CGDisplayMoveCursorToPoint(displayId, point);
#endif
    
    getEventQueue()->mouseWarped(x,y);
}



// ----------------------------------------------------------------------------------------------------------
// useCursor
// ----------------------------------------------------------------------------------------------------------

void GraphicsWindowCocoa::useCursor(bool cursorOn)
{
    if (_traits.valid())
        _traits->useCursor = cursorOn;
    DarwinWindowingSystemInterface* wsi = dynamic_cast<DarwinWindowingSystemInterface*>(osg::GraphicsContext::getWindowingSystemInterface());
    if (wsi == NULL) {
        OSG_WARN << "GraphicsWindowCarbon::useCursor :: could not get OSXCarbonWindowingSystemInterface" << std::endl;
        return;
    }

    CGDirectDisplayID displayId = wsi->getDisplayID((*_traits));
    CGDisplayErr err = kCGErrorSuccess;

    if (cursorOn) {
        err = CGDisplayShowCursor(displayId);
    } else {
        err = CGDisplayHideCursor(displayId);
    }

    if (err != kCGErrorSuccess) {
        OSG_WARN << "GraphicsWindowCocoa::useCursor failed with " << err << std::endl;
    }
}


// ----------------------------------------------------------------------------------------------------------
// setCursor
// ----------------------------------------------------------------------------------------------------------

void GraphicsWindowCocoa::setCursor(MouseCursor mouseCursor)
{
    if (_currentCursor == mouseCursor) {
        return;
    }

    NSAutoreleasePool* localPool = [[NSAutoreleasePool alloc] init];
    switch (mouseCursor)
    {

        case NoCursor:
            [NSCursor hide];
            break;

        case LeftArrowCursor:
        case RightArrowCursor:
            [[NSCursor arrowCursor] set];
            break;

        case TextCursor:
            [[NSCursor IBeamCursor] set];
            break;

        case CrosshairCursor:
            [[NSCursor crosshairCursor] set];
            break;

        default:
            OSG_INFO << "GraphicsWindowCocoa::setCursor :: unsupported MouseCursor: " << mouseCursor << std::endl;
    }

    if (_currentCursor == NoCursor) {
        [NSCursor unhide];
    }

    _currentCursor = mouseCursor;
    [localPool release];
}


// ----------------------------------------------------------------------------------------------------------
// setSyncToVBlank
// ----------------------------------------------------------------------------------------------------------

void GraphicsWindowCocoa::setSyncToVBlank(bool f)
{
    if (_traits.valid()) _traits->vsync = f;
    GLint VBL(f?1:0);
    [_context setValues:&VBL forParameter:NSOpenGLCPSwapInterval];
}


bool GraphicsWindowCocoa::isMultiTouchEnabled()
{
    return _multiTouchEnabled;
}
void GraphicsWindowCocoa::setMultiTouchEnabled(bool b)
{
    _multiTouchEnabled = b;
    
#if defined(MAC_OS_X_VERSION_10_6) && (MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_6)
    if (_view) [_view setAcceptsTouchEvents: b];
#else
    if (b) {
        OSG_WARN << "GraphicsWindowCocoa :: multi-touch only available for OS X >= 10.6, please check your compile settings" << std::endl;
    }
#endif
}


// ----------------------------------------------------------------------------------------------------------
// d'tor
// ----------------------------------------------------------------------------------------------------------

GraphicsWindowCocoa::~GraphicsWindowCocoa()
{
    close();
}



#pragma mark CocoaWindowingSystemInterface

// ----------------------------------------------------------------------------------------------------------
// CocoaWindowingSystemInterface
// ----------------------------------------------------------------------------------------------------------

struct CocoaWindowingSystemInterface : public DarwinWindowingSystemInterface
{

    CocoaWindowingSystemInterface() : DarwinWindowingSystemInterface()
    {
    }

    void initAsStandaloneApplication()
    {
        _init();

        static bool s_inited = false;
        if (s_inited) return;
        s_inited = true;

        OSG_INFO << "CocoaWindowingSystemInterface::initAsStandaloneApplication " << std::endl;

        ProcessSerialNumber psn;
        if (!GetCurrentProcess(&psn)) {
            TransformProcessType(&psn, kProcessTransformToForegroundApplication);
            SetFrontProcess(&psn);
        }

        NSAutoreleasePool* localPool = [[NSAutoreleasePool alloc] init];

        if (NSApp == nil) {
            [NSApplication sharedApplication];
        }

        [NSApp setDelegate: [[CocoaAppDelegate alloc] init] ];

        createApplicationMenus();

        [NSApp finishLaunching];

        [localPool release];
    }

    virtual osg::GraphicsContext* createGraphicsContext(osg::GraphicsContext::Traits* traits)
    {
        _init();

        if (!traits->pbuffer)
        {
            GraphicsWindowCocoa::WindowData* windowData = traits->inheritedWindowData ? dynamic_cast<GraphicsWindowCocoa::WindowData*>(traits->inheritedWindowData.get()) : NULL;

            if (!windowData || (windowData && windowData->poseAsStandaloneApp()))
            {
                initAsStandaloneApplication();
            }
        }

        return createGraphicsContextImplementation<PixelBufferCocoa, GraphicsWindowCocoa>(traits);
    }

    virtual ~CocoaWindowingSystemInterface()
    {
    }

private:
    NSString *getApplicationName(void)
    {
        NSDictionary *dict;
        NSString *appName = 0;

        /* Determine the application name */
        dict = (NSDictionary *)CFBundleGetInfoDictionary(CFBundleGetMainBundle());
        if (dict)
            appName = [dict objectForKey: @"CFBundleName"];

        if (![appName length])
            appName = [[NSProcessInfo processInfo] processName];

        return appName;
    }

    void createApplicationMenus(void)
    {
        NSString *appName;
        NSString *title;
        NSMenu *appleMenu;
        NSMenuItem *menuItem;

        /* Create the main menu bar */
        [NSApp setMainMenu:[[NSMenu alloc] init]];

        /* Create the application menu */
        appName = getApplicationName();
        appleMenu = [[NSMenu alloc] initWithTitle:@""];

        /* Add menu items */
        title = [@"About " stringByAppendingString:appName];
        [appleMenu addItemWithTitle:title action:@selector(orderFrontStandardAboutPanel:) keyEquivalent:@""];

        [appleMenu addItem:[NSMenuItem separatorItem]];

        NSMenu* service_menu = [[NSMenu alloc] init];
        NSMenuItem* service_menu_item = [[NSMenuItem alloc] initWithTitle:@"Services" action:nil keyEquivalent:@""];
        [service_menu_item setSubmenu: service_menu];
        [appleMenu addItem: service_menu_item];
        [NSApp setServicesMenu: service_menu];

        [appleMenu addItem:[NSMenuItem separatorItem]];

        title = [@"Hide " stringByAppendingString:appName];
        [appleMenu addItemWithTitle:title action:@selector(hide:) keyEquivalent:@/*"h"*/"h"];

        menuItem = (NSMenuItem *)[appleMenu addItemWithTitle:@"Hide Others" action:@selector(hideOtherApplications:) keyEquivalent:@/*"h"*/""];
        [menuItem setKeyEquivalentModifierMask:(NSAlternateKeyMask|NSCommandKeyMask)];

        [appleMenu addItemWithTitle:@"Show All" action:@selector(unhideAllApplications:) keyEquivalent:@""];

        [appleMenu addItem:[NSMenuItem separatorItem]];

        title = [@"Quit " stringByAppendingString:appName];
        [appleMenu addItemWithTitle:title action:@selector(terminate:) keyEquivalent:@/*"q"*/"q"];

        /* Put menu into the menubar */
        menuItem = [[NSMenuItem alloc] initWithTitle:@"" action:nil keyEquivalent:@""];
        [menuItem setSubmenu:appleMenu];
        [[NSApp mainMenu] addItem:menuItem];
        [menuItem release];

        /* Tell the application object that this is now the application menu */
        [NSApp setAppleMenu:appleMenu];
        [appleMenu release];


    }

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
