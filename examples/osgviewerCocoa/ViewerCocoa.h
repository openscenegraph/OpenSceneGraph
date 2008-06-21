/* -*-c++-*-
* 
*  OpenSceneGraph example, osgviewerCacoa.
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is
*  furnished to do so, subject to the following conditions:
*
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
*  THE SOFTWARE.
*/

//
//  ViewerCocoa.h
//  osgviewerCocoa
//
//  Created by Eric Wing on 11/12/06.
//  Copyright 2006. Released under the OSGPL.
//  Ported to osgViewer::Viewer by Martin Lavery 7/06/07
//
/* This is the class interface for a custom NSView that interfaces with an osgViewer.
 * Because Cocoa is written in Objective-C, but OSG is written in C++, we rely on
 * Objective-C++ to make the integration easy.
 *
 * One thing to remember is that any Objective-C files that include this header
 * must also be compiled as Objective-C++ because there are C++ constructs in
 * this file (such as namespaces) which the normal Objective-C compiler 
 * won't understand. (The easy way to do this is rename the .m files to .mm.) 
 *
 * In the event that you have a large, pre-existing code base written in
 * pure Objective-C and you find that the header include propagates to a
 * large number of your files, forcing you to mark many files to be compiled as
 * Objective-C++, and you find that you don't want to change these files because
 * they are shared with other pure Obj-C projects, you might consider further 
 * wrapping the C++ code so there are only C or Obj-C constructs in 
 * this header. There are several different techniques ranging from, wrapping 
 * the C++ code in pure C interfaces, to simply using void pointers in this

* file for any C++ pointers and casting appropriately in the implementation
 * file.
 */


#import <Cocoa/Cocoa.h>

namespace osgViewer
{
    // Just a forward declaration so I don't need the #include in the header.
    class Viewer;
    class GraphicsWindowEmbedded;
}

// Subclass NSOpenGLView. We could subclass NSView instead, but NSOpenGLView is easy.
@interface ViewerCocoa : NSOpenGLView
{
    // Note: In Objective-C++, if you use objects instead of pointers as
    // member instance variables, you MUST turn on "Call C++ Default Ctors/Dtors in Objective-C".
    // -fobjc-call-cxx-cdtors
    // This option makes sure constructors and destructors are run.
    // This option is only available for gcc 4.0+ (Mac OS X 10.4+)

    // Is SimpleViewer supposed use ref_ptr? (Doesn't look like it to me.)
    // If so, remember ref_ptr is an object on the stack and the cdtors option must be activated.
    // We could also make simpleViewer an object instead of a pointer, but again, turn on the option.
    osgViewer::Viewer* theViewer;
    osgViewer::GraphicsWindowEmbedded* graphicsWindow;
        

    // This timer is used to trigger animation callbacks since everything is event driven.
    NSTimer* animationTimer;

    // Flags to help track whether ctrl-clicking or option-clicking is being used
    BOOL isUsingCtrlClick;
    BOOL isUsingOptionClick;
    
    // Flag to track whether the OpenGL multithreading engine is enabled or not
    BOOL isUsingMultithreadedOpenGLEngine;
    
}

// My custom static method to create a basic pixel format
+ (NSOpenGLPixelFormat*) basicPixelFormat;


// Official init methods
- (id) initWithFrame:(NSRect)frame_rect pixelFormat:(NSOpenGLPixelFormat*)pixel_format;
- (id) initWithCoder:(NSCoder*)the_coder;
- (id) initWithFrame:(NSRect)frame_rect;

// Official function, overridden by this class to prevent flashing/tearing when in splitviews, scrollviews, etc.
- (void) renewGState;

// My custom function for minimization.
- (void) prepareForMiniaturization:(NSNotification*)notification;


// Custom function to allow users to know if the Multithreaded OpenGL Engine is enabled
- (BOOL) isUsingMultithreadedOpenGLEngine;

// Private init helper methods
- (void) initSharedOpenGLContext;
- (void) commonInit;
- (void) initOSGViewer;
- (void) initAnimationTimer;

// Official/Special NSOpenGLView method that gets called for you to prepare your OpenGL state.
- (void) prepareOpenGL;
// Class dealloc method
- (void) dealloc;
- (void) finalize;

// Official mouse event methods
- (void) mouseDown:(NSEvent*)the_event;
- (void) mouseDragged:(NSEvent*)the_event;
- (void) mouseUp:(NSEvent*)the_event;
- (void) rightMouseDown:(NSEvent*)the_event;
- (void) rightMouseDragged:(NSEvent*)the_event;
- (void) rightMouseUp:(NSEvent*)the_event;
- (void) otherMouseDown:(NSEvent*)the_event;
- (void) otherMouseDragged:(NSEvent*)the_event;
- (void) otherMouseUp:(NSEvent*)the_event;

// Private setter/getter methods to track ctrl/option-clicking
- (void) setIsUsingCtrlClick:(BOOL)is_using_ctrl_click;
- (BOOL) isUsingCtrlClick;
- (void) setIsUsingOptionClick:(BOOL)is_using_option_click;
- (BOOL) isUsingOptionClick;
// Private helper methods to help deal with mouse events
- (void) doLeftMouseButtonDown:(NSEvent*)the_event;
- (void) doLeftMouseButtonUp:(NSEvent*)the_event;
- (void) doRightMouseButtonDown:(NSEvent*)the_event;
- (void) doRightMouseButtonUp:(NSEvent*)the_event;
- (void) doMiddleMouseButtonDown:(NSEvent*)the_event;
- (void) doExtraMouseButtonDown:(NSEvent*)the_event buttonNumber:(int)button_number;
- (void) doMiddleMouseButtonUp:(NSEvent*)the_event;
- (void) doExtraMouseButtonUp:(NSEvent*)the_event buttonNumber:(int)button_number;

// Official scrollWheel (scrollball) method
- (void) scrollWheel:(NSEvent*)the_event;

// Official methods for keyboard events
- (BOOL) acceptsFirstResponder;
- (void) keyDown:(NSEvent*)the_event;
- (void) keyUp:(NSEvent*)the_event;

// My custom method to handle timer callbacks
- (void) animationCallback;

// Official methods for view stuff and drawing
- (BOOL) isOpaque;
- (void) resizeViewport;
- (void) reshape;
- (void) drawRect:(NSRect)the_rect;

// Private helper methods for drawing
- (NSBitmapImageRep*) renderOpenGLSceneToFramebuffer;
- (NSBitmapImageRep*) renderOpenGLSceneToFramebufferAsFormat:(int)gl_format viewWidth:(float)view_width viewHeight:(float)view_height;
- (NSBitmapImageRep*) renderOpenGLSceneToFramebufferAsFormat:(int)gl_format viewWidth:(float)view_width viewHeight:(float)view_height clearColorRed:(float)clear_red clearColorGreen:(float)clear_green clearColorBlue:(float)clear_blue clearColorAlpha:(float)clear_alpha;
- (NSImage*)imageFromBitmapImageRep:(NSBitmapImageRep*)bitmap_image_rep;


// Official methods for drag and drop (view as target)
- (unsigned int) draggingEntered:(id <NSDraggingInfo>)the_sender;
- (void) draggingExited:(id <NSDraggingInfo>)the_sender;
- (BOOL) prepareForDragOperation:(id <NSDraggingInfo>)the_sender;
- (BOOL) performDragOperation:(id <NSDraggingInfo>)the_sender;
- (void) concludeDragOperation:(id <NSDraggingInfo>)the_sender;

// Official method for copy (i.e. copy & paste)
- (IBAction) copy:(id)sender;

// Private helper methods for drag and drop and copy/paste (view as source)
- (NSData*) dataWithTIFFOfContentView;
- (NSData*) contentsAsDataOfType:(NSString *)pboardType;
- (void) startDragAndDropAsSource:(NSEvent*)the_event;


// Examples of providing an action to connect to.
- (IBAction) resetPosition:(id)the_sender;
- (IBAction) takeBackgroundColorFrom:(id)the_sender;
- (IBAction) toggleFullScreen:(id)the_sender;


@end

