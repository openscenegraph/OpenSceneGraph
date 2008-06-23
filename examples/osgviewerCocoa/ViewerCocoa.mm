/* OpenSceneGraph example, osgviewerCacoa.
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
//  ViewerCocoa.mm
//  osgviewerCocoa
//
//  Created by Eric Wing on 11/12/06.
//  Copyright 2006. Released under the OSGPL.
//  Ported to osgViewer::Viewer by Martin Lavery 7/06/07
//
/* This class demonstrates how to subclass NSOpenGLView to integrate with the 
 * osgViewer::Viewer.
 * This class demonstrates:
 * Objective-C++
 * How to subclass NSOpenGLView
 * Specifying OpenGL pixel formats
 * V-sync
 * Timer based animation
 * One button and multibutton mice.
 * Scroll events (Mighty Mouse, Two-finger trackpads)
 * Keyboard events
 * Drag and drop (as target)
 * Drag and drop (as source)
 * Copy (for copy/paste)
 * Resolution Independent UI (maybe, not Leopard tested/only Tiger Quartz Debug)
 * Target-Action (for other widgets to invoke actions on this view)
 * Shared OpenGLContexts
 * Multithreaded OpenGL Engine (with CPU count check)
 * Getting an image for minimization in the Dock
 * Using osg::Camera's Framebuffer Objects to generate screen captures
 * Code to detect drawing to offscreen (e.g. printer)
 * Localization/Localizable strings (really need some different languages)
 * Use of respondsToSelector and instancesRespondToSelector to demonstrate 
 *   runtime feature checking to provide access to newer features while still
 *   supporting legacy versions.
 * 
 * Things not demonstrated by this view or application example (but would be interesting):
 * Cocoa Bindings (highly recommended)
 * Core Data (works great with Cocoa Bindings)
 * Custom Interface Builder palette with Inspector configurable options
 * More PixelFormat options
 * Fullscreen mode
 * Low-level CGL access (kind of have with multithreaded OpenGL engine).
 * Delegates for your view
 * Multithreading
 * Updating images of the window for the minimized view in the Dock
 * Full Printing support
 * Non-view stuff (Application Delegates, models, controllers)
 * Launching by double-clicking a model or drag-and-drop onto Application Icon (non-view)
 * Launching via commandline with parameters (non-view)
 */ 

/* Commentary:
 * This class packs a lot of functionality that you might not expect from a "Viewer".
 * The reason is that Mac users have high expectations of their applications, so things 
 * that might not even be considered on other platforms (like drag-and-drop), are almost 
 * a requirment on Mac OS X.
 * The good news is that this class can almost be used as a template for other purposes.
 * If you are looking for the bare minimum code needed, focus your attention on 
 * the init* routines and drawRect.
 */

/*
 * Coding conventions:
 * My coding style is slightly different than what you normally see in Cocoa.
 * So here is the cheat sheet:
 * I hate Hungarian (Microsoft) notation. And prefixed underscore _variables 
 * are technically reserved by the compiler which is why I avoid them. So...
 * Member variables use camelCase (Pascal/Java)
 * Local variables use under_scores (Ada)
 * For methods, I follow standard Cocoa conventions.
 * I tend to keep * with the type (e.g. NSView* foo) instead of with the variable (NSView *foo).
 * (I tend to think of the pointer as part of the type.)
 * For Obj-C named parameters, I tend to keep the namedParameter and the value 
 * together instead of separated by spaces 
 * e.g. [self initWithX:x_val yVal:y_val zVal:z_val].
 * (When I was first learning Objective-C, this made it easier for me to 
 * figure out which things were paired.)
 */
#import "ViewerCocoa.h"

#include <OpenGL/gl.h>
#include <OpenGL/glu.h> // handy for gluErrorString

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgGA/TrackballManipulator>
// Needed to explicitly typecast keys to the OSG type
#include <osgGA/GUIEventAdapter>
// Needed to load models
#include <osgDB/ReadFile>

// Used so I can change the background (clear) color

#include <osg/Vec4>

// For debugging
//#include <osg/Notify>

// osgText is used only to announce that you can use drag-and-drop to load models.
// osgViewer itself does not have a dependency on osgText.
#include <osg/ref_ptr>
#include <osgText/Text>

#include <osg/Geode>

// Needed for Multithreaded OpenGL Engine (sysctlbyname for num CPUs)
#include <sys/types.h>
#include <sys/sysctl.h>
#include <OpenGL/OpenGL.h> // for CoreOpenGL (CGL) for Multithreaded OpenGL Engine



// This is optional. This allows memory for things like textures and displaylists to be shared among different contexts.
#define VIEWER_USE_SHARED_CONTEXTS
#ifdef VIEWER_USE_SHARED_CONTEXTS
static NSOpenGLContext* s_sharedOpenGLContext = NULL;
#endif // VIEWER_USE_SHARED_CONTEXTS

// Taken/Adapted from one of the Apple OpenGL developer examples
static void Internal_SetAlpha(NSBitmapImageRep *imageRep, unsigned char alpha_value)
{
    register unsigned char * sp = [imageRep bitmapData];
    register int bytesPerRow = [imageRep bytesPerRow];
    register int height = [imageRep pixelsHigh];
    register int width = [imageRep pixelsWide];

    for(int i=0; i<height; i++)
    {
        register unsigned int * the_pixel = (unsigned int *) sp;
        register int w = width;
        while (w-- > 0)
        {
            unsigned char* sp_char = (unsigned char *) the_pixel;
//            register unsigned char * the_red = sp_char;
//            register unsigned char * the_green = (sp_char+1);
//            register unsigned char * the_blue = (sp_char+2);
            register unsigned char * the_alpha = (sp_char+3);
    
            *the_alpha = alpha_value;
            *the_pixel++;
        }
        sp += bytesPerRow;
    }
}


@implementation ViewerCocoa

// My simple pixel format definition
+ (NSOpenGLPixelFormat*) basicPixelFormat
{
    NSOpenGLPixelFormatAttribute pixel_attributes[] =
    {
        NSOpenGLPFAWindow,
        NSOpenGLPFADoubleBuffer,  // double buffered
        NSOpenGLPFADepthSize, (NSOpenGLPixelFormatAttribute)32, // depth buffer size in bits
//        NSOpenGLPFAColorSize, (NSOpenGLPixelFormatAttribute)24, // Not sure if this helps
//        NSOpenGLPFAAlphaSize, (NSOpenGLPixelFormatAttribute)8, // Not sure if this helps
        (NSOpenGLPixelFormatAttribute)nil
    };
    return [[[NSOpenGLPixelFormat alloc] initWithAttributes:pixel_attributes] autorelease];
}



////////////////////////////////////////////////////////////////////////
/////////////////////////// Init Stuff /////////////////////////////////
////////////////////////////////////////////////////////////////////////

/* This is the designated initializer for an NSOpenGLView. However, since I'm 
 * using Interface Builder to help, initWithCoder: is the initializer that gets called.
 * But for completeness, I implement this method here.
 */
- (id) initWithFrame:(NSRect)frame_rect pixelFormat:(NSOpenGLPixelFormat*)pixel_format
{
    self = [super initWithFrame:frame_rect pixelFormat:pixel_format];
    if(self)
    {
        [self commonInit];
    }
    return self;
}

/* Going through the IB palette, this initializer is calling instead of the designated initializer
 * initWithFrame:pixelFormat: 
 * But for some reason, the pixel format set in IB selected seems to be either ignored or is missing
 * a value I need. (The depth buffer looks too shallow to me and glErrors are triggered.)
 * So I explicitly set the pixel format inside here (overriding the IB palette options).
 * This probably should be investigated, but since IB is getting an overhaul for Leopard,
 * I'll wait on this for now.
 */
- (id) initWithCoder:(NSCoder*)the_coder
{
    self = [super initWithCoder:the_coder];
    if(self)
    {
        NSOpenGLPixelFormat* pixel_format = [ViewerCocoa basicPixelFormat];
        [self setPixelFormat:pixel_format];
        [self commonInit];
    }
    return self;
}

/* Some generic code expecting regular NSView's may call this initializer instead of the specialized NSOpenGLView designated initializer.
 * I override this method here to make sure it does the right thing.
 */
- (id) initWithFrame:(NSRect)frame_rect
{
    self = [super initWithFrame:frame_rect pixelFormat:[ViewerCocoa basicPixelFormat]];
    if(self)
    {
        [self commonInit];
    }
    return self;
}


// My custom methods to centralize common init stuff
- (void) commonInit
{
    isUsingCtrlClick = NO;
    isUsingOptionClick = NO;
    isUsingMultithreadedOpenGLEngine = NO;

    [self initSharedOpenGLContext];

    [self initOSGViewer];
    [self initAnimationTimer];
    
    // Register for Drag and Drop
    [self registerForDraggedTypes:[NSArray arrayWithObjects:NSFilenamesPboardType, NSURLPboardType, nil]];
    
    // Add minification observer so we can set the Dock picture since OpenGL views don't do this automatically for us.
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(prepareForMiniaturization:) name:NSWindowWillMiniaturizeNotification object:nil];

}

/* Optional: This will setup shared OpenGL contexts so resources like textures, etc, can be shared/reused
 * by multiple instances of SimpleViwerCocoa views.
 */
- (void) initSharedOpenGLContext
{
#ifdef VIEWER_USE_SHARED_CONTEXTS

    NSOpenGLContext* this_views_opengl_context = nil;
    
    // create a context the first time through
    if(s_sharedOpenGLContext == NULL)
    {
        s_sharedOpenGLContext = [[NSOpenGLContext alloc]
                      initWithFormat:[ViewerCocoa basicPixelFormat]
                        shareContext:nil];
        
    }
    
    this_views_opengl_context = [[NSOpenGLContext alloc]
                      initWithFormat:[ViewerCocoa basicPixelFormat]
                        shareContext:s_sharedOpenGLContext];
     [self setOpenGLContext:this_views_opengl_context];
//    [this_views_opengl_context makeCurrentContext];
#endif // VIEWER_USE_SHARED_CONTEXTS
}

// Allocate a Viewer and do basic initialization. No assumption about having an
// a valid OpenGL context is made by this function.
- (void) initOSGViewer
{


//    osg::setNotifyLevel( osg::DEBUG_FP );
    theViewer = new osgViewer::Viewer;
    graphicsWindow = theViewer->setUpViewerAsEmbeddedInWindow(0,0,740,650); // numbers from Nib
    // Builts in Stats handler
    theViewer->addEventHandler(new osgViewer::StatsHandler);
#ifdef VIEWER_USE_SHARED_CONTEXTS
    // Workaround: osgViewer::Viewer automatically increments its context ID values.
    // Since we're using a shared context, we want all Viewer's to use the same context ID.
    // There is no API to avoid this behavior, so we need to undo what Viewer's constructor did.
    graphicsWindow->getState()->setContextID(0);
    osg::DisplaySettings::instance()->setMaxNumberOfGraphicsContexts(1);
#endif // VIEWER_USE_SHARED_CONTEXTS

    // Cocoa follows the same coordinate convention as OpenGL. osgViewer's default is inverted.
    theViewer->getEventQueue()->getCurrentEventState()->setMouseYOrientation(osgGA::GUIEventAdapter::Y_INCREASING_UPWARDS);
    // Use a trackball manipulator...matches nicely with the Mighty Mouse Scrollball.
    theViewer->setCameraManipulator(new osgGA::TrackballManipulator);
    
}

- (void) initAnimationTimer
{
    // Cocoa is event driven, so by default, there is nothing to trigger redraws for animation.
    // The easiest way to animate is to set a repeating NSTimer which triggers a redraw.
    SEL the_selector;
    NSMethodSignature* a_signature;
    NSInvocation* an_invocation;
    // animationCallback is my animation callback method
    the_selector = @selector( animationCallback );
    a_signature = [ViewerCocoa instanceMethodSignatureForSelector:the_selector];
    an_invocation = [NSInvocation invocationWithMethodSignature:a_signature] ;
    [an_invocation setSelector:the_selector];
    [an_invocation setTarget:self];
    
    animationTimer = [NSTimer
        scheduledTimerWithTimeInterval:1.0/60.0 // fps
        invocation:an_invocation
        repeats:YES];
    [animationTimer retain];
    
    // For single threaded apps like this one,
    // Cocoa seems to block timers or events sometimes. This can be seen
    // when I'm animating (via a timer) and you open an popup box or move a slider.
    // Apparently, sheets and dialogs can also block (try printing).
    // To work around this, Cocoa provides different run-loop modes. I need to 
    // specify the modes to avoid the blockage.
    // NSDefaultRunLoopMode seems to be the default. I don't think I need to explicitly
    // set this one, but just in case, I will set it anyway.
    [[NSRunLoop currentRunLoop] addTimer:animationTimer forMode:NSDefaultRunLoopMode];
    // This seems to be the one for preventing blocking on other events (popup box, slider, etc)
    [[NSRunLoop currentRunLoop] addTimer:animationTimer forMode:NSEventTrackingRunLoopMode];
    // This seems to be the one for dialogs.
    [[NSRunLoop currentRunLoop] addTimer:animationTimer forMode:NSModalPanelRunLoopMode];
}

- (void) dealloc
{
    [animationTimer invalidate];
    [animationTimer release];
    delete theViewer;
    theViewer = NULL;
    [super dealloc];
}

- (void) finalize
{
    delete theViewer;
    theViewer = NULL;
    [super finalize];
}

/* NSOpenGLView defines this method to be called (only once) after the OpenGL
 * context is created and made the current context. It is intended to be used to setup
 * your initial OpenGL state. This seems like a good place to initialize the 
 * OSG stuff. This method exists in 10.3 and later. If you are running pre-10.3, you
 * must manually call this method sometime after the OpenGL context is created and 
 * made current, or refactor this code.
 */
- (void) prepareOpenGL
{
    [super prepareOpenGL];
    
    // The NSOpenGLCPSwapInterval seems to be vsync. If 1, buffers are swapped with vertical refresh.
    // If 0, flushBuffer will execute as soon as possible.
    const GLint swap_interval = 1 ;
    [[self openGLContext] setValues:&swap_interval forParameter:NSOpenGLCPSwapInterval];


    // Try new multithreaded OpenGL engine?
    // See Technical Note TN2085 Enabling multi-threaded execution of the OpenGL framework
    // http://developer.apple.com/technotes/tn2006/tn2085.html
    // For this simple viewer, you are probably not going to see a speed boost, but possibly a speed hit,
    // since we probably don't have much data to dispatch,
    // but it is enabled anyway for demonstration purposes.
    uint64_t num_cpus = 0;
    size_t num_cpus_length = sizeof(num_cpus);
    // Multithreaded engine only benefits with muliple CPUs, so do CPU count check
    // I've been told that Apple has started doing this check behind the scenes in some version of Tiger.
    if(sysctlbyname("hw.activecpu", &num_cpus, &num_cpus_length, NULL, 0) == 0)
    {
//        NSLog(@"Num CPUs=%d", num_cpus);
        if(num_cpus >= 2)
        {
            // Cleared to enable multi-threaded engine
            // kCGLCEMPEngine may not be defined before certain versions of Tiger/Xcode,
            // so use the numeric value 313 instead to keep things compiling.
            CGLError error_val = CGLEnable((CGLContextObj)[[self openGLContext] CGLContextObj], static_cast<CGLContextEnable>(313));
            if(error_val != 0)
            {
                // The likelihood of failure seems quite high on older hardware, at least for now.
                // NSLog(@"Failed to enable Multithreaded OpenGL Engine: %s", CGLErrorString(error_val));
                isUsingMultithreadedOpenGLEngine = NO;
            }
            else
            {
                // NSLog(@"Success! Multithreaded OpenGL Engine activated!");
                isUsingMultithreadedOpenGLEngine = YES;
            }
        }
        else
        {
            isUsingMultithreadedOpenGLEngine = NO;
        }
    }

    // This is also might be a good place to setup OpenGL state that OSG doesn't control.
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

/*
    GLint maxbuffers[1];
    glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS_EXT, maxbuffers);
    NSLog(@"GL_MAX_COLOR_ATTACHMENTS=%d", maxbuffers[0]);
*/

    // We need to tell the osgViewer what the viewport size is
    [self resizeViewport];


    // This is optional:
    // This is to setup some default text in the OpenGL view so the 
    // user knows that they should drag and drop a model into the view.
    osg::ref_ptr<osgText::Text> default_text = new osgText::Text;

    default_text->setAlignment(osgText::Text::CENTER_CENTER);
    default_text->setBackdropType(osgText::Text::OUTLINE);
//    default_text->setBackdropImplementation(osgText::Text::POLYGON_OFFSET);
    default_text->setColor(osg::Vec4(1.0, 1.0, 0.0, 1.0));
    default_text->setBackdropColor(osg::Vec4(0.0, 0.0, 0.0, 1.0));
    default_text->setAxisAlignment(osgText::Text::XZ_PLANE);
    
    // We should use a (Cocoa) localizable string instead of a hard coded string.
//    default_text->setText("Drag-and-Drop\nyour .osg model here!");
    // The first string is the key name (you need a Localizable.strings file for your Nib). The second string is just a comment.
    NSString* localized_string = NSLocalizedString(@"DragAndDropHere", @"Drag-and-Drop\nyour .osg model here!");
    default_text->setText([localized_string UTF8String]);
    
    osg::ref_ptr<osg::Geode> the_geode = new osg::Geode;
    the_geode->addDrawable(default_text.get());

    theViewer->setSceneData(the_geode.get());
}

/* disableScreenUpdatesUntilFlush was introduced in Tiger. It will prevent
 * unnecessary screen flashing caused by NSSplitViews or NSScrollviews.
 * From Apple's release notes:
 
NSWindow -disableScreenUpdatesUntilFlush API (Section added since WWDC)

When a view that renders to a hardware surface (such as an OpenGL view) is placed in an NSScrollView or NSSplitView, there can be a noticeable flicker or lag when the scroll position or splitter position is moved. This happens because each move of the hardware surface takes effect immediately, before the remaining window content has had the chance to draw and flush.

To enable applications to eliminate this visual artifact, Tiger AppKit provides a new NSWindow message, -disableScreenUpdatesUntilFlush. This message asks a window to suspend screen updates until the window's updated content is subsequently flushed to the screen. This message can be sent from a view that is about to move its hardware surface, to insure that the hardware surface move and window redisplay will be visually synchronized. The window responds by immediately disabling screen updates via a call to NSDisableScreenUpdates(), and setting a flag that will cause it to call NSEnableScreenUpdates() later, when the window flushes. It is permissible to send this message to a given window more than once during a given window update cycle; the window will only suspend and re-enable updates once during that cycle.

A view class that renders to a hardware surface can send this message from an override of -renewGState (a method that is is invoked immediately before the view's surface is moved) to effectively defer compositing of the moved surface until the window has finished drawing and flushing its remaining content.
A -respondsToSelector: check has been used to provide compatibility with previous system releases. On pre-Tiger systems, where NSWindow has no -disableScreenUpdatesUntilFlush method, the -renewGState override will have no effect.
 */
- (void) renewGState
{
    NSWindow* the_window = [self window];
    if([the_window respondsToSelector:@selector(disableScreenUpdatesUntilFlush)])
    {
        [the_window disableScreenUpdatesUntilFlush];
    }
    [super renewGState];
}


/* When you minimize an app, you usually can see its shrunken contents 
 * in the dock. However, an OpenGL view by default only produces a blank
 * white window. So we use this method to do an image capture of our view
 * which will be used as its minimized picture.
 * (A possible enhancement to consider is to update the picture over time.)
 */
- (void) prepareForMiniaturization:(NSNotification*)notification
{
    [[self openGLContext] makeCurrentContext];
    NSBitmapImageRep* ns_image_rep = [self renderOpenGLSceneToFramebuffer];
    if([self lockFocusIfCanDraw])
    {
        [ns_image_rep draw];
        [self unlockFocus];
        [[self window] flushWindow];
    }
}

/* Allow people to easily query if the multithreaded OpenGL engine is activated.
 */
- (BOOL) isUsingMultithreadedOpenGLEngine
{
    return isUsingMultithreadedOpenGLEngine;
}


////////////////////////////////////////////////////////////////////////
/////////////////////////// End Init Stuff /////////////////////////////
////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////
/////////////////////////// Mouse Stuff ////////////////////////////////
////////////////////////////////////////////////////////////////////////

- (void) mouseDown:(NSEvent*)the_event
{
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
    else if([the_event modifierFlags] & NSCommandKeyMask)
    {
        [self startDragAndDropAsSource:the_event];
    }
    else
    {
        [self doLeftMouseButtonDown:the_event];
    }
}

- (void) mouseDragged:(NSEvent*)the_event
{
    // We must convert the mouse event locations from the window coordinate system to the
    // local view coordinate system.
    NSPoint the_point = [the_event locationInWindow];
    NSPoint converted_point = [self convertPoint:the_point fromView:nil];
    
    theViewer->getEventQueue()->mouseMotion(converted_point.x, converted_point.y);
    [self setNeedsDisplay:YES];
}

- (void) mouseUp:(NSEvent*)the_event
{
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
    [self doRightMouseButtonDown:the_event];
}

- (void) rightMouseDragged:(NSEvent*)the_event
{
    // We must convert the mouse event locations from the window coordinate system to the
    // local view coordinate system.
    NSPoint the_point = [the_event locationInWindow];
    NSPoint converted_point = [self convertPoint:the_point fromView:nil];
    
    theViewer->getEventQueue()->mouseMotion(converted_point.x, converted_point.y);
    [self setNeedsDisplay:YES];
}

- (void) rightMouseUp:(NSEvent*)the_event
{
    [self doRightMouseButtonUp:the_event];
}

// "otherMouse" seems to capture middle button and any other buttons beyond (4th, etc).
- (void) otherMouseDown:(NSEvent*)the_event
{
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
    // We must convert the mouse event locations from the window coordinate system to the
    // local view coordinate system.
    NSPoint the_point = [the_event locationInWindow];
    NSPoint converted_point = [self convertPoint:the_point fromView:nil];
    
    theViewer->getEventQueue()->mouseMotion(converted_point.x, converted_point.y);
    [self setNeedsDisplay:YES];
}

// "otherMouse" seems to capture middle button and any other buttons beyond (4th, etc).
- (void) otherMouseUp:(NSEvent*)the_event
{
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
    }    [self doExtraMouseButtonUp:the_event buttonNumber:[the_event buttonNumber]];
}

- (void) setIsUsingCtrlClick:(BOOL)is_using_ctrl_click
{
    isUsingCtrlClick = is_using_ctrl_click;
}

- (BOOL) isUsingCtrlClick
{
    return isUsingCtrlClick;
}

- (void) setIsUsingOptionClick:(BOOL)is_using_option_click
{
    isUsingOptionClick = is_using_option_click;
}

- (BOOL) isUsingOptionClick
{
    return isUsingOptionClick;
}


- (void) doLeftMouseButtonDown:(NSEvent*)the_event
{
    // We must convert the mouse event locations from the window coordinate system to the
    // local view coordinate system.
    NSPoint the_point = [the_event locationInWindow];
    NSPoint converted_point = [self convertPoint:the_point fromView:nil];
    if([the_event clickCount] == 1)
    {
        theViewer->getEventQueue()->mouseButtonPress(converted_point.x, converted_point.y, 1);
    }
    else
    {
        theViewer->getEventQueue()->mouseDoubleButtonPress(converted_point.x, converted_point.y, 1);
        
        
        // Let's toggle fullscreen for show
        [self toggleFullScreen:nil];

    }
    [self setNeedsDisplay:YES];
}

- (void) doLeftMouseButtonUp:(NSEvent*)the_event
{
    // We must convert the mouse event locations from the window coordinate system to the
    // local view coordinate system.
    NSPoint the_point = [the_event locationInWindow];
    NSPoint converted_point = [self convertPoint:the_point fromView:nil];
    
    theViewer->getEventQueue()->mouseButtonRelease(converted_point.x, converted_point.y, 1);
    [self setNeedsDisplay:YES];
}

- (void) doRightMouseButtonDown:(NSEvent*)the_event
{
    // We must convert the mouse event locations from the window coordinate system to the
    // local view coordinate system.
    NSPoint the_point = [the_event locationInWindow];
    NSPoint converted_point = [self convertPoint:the_point fromView:nil];
    if([the_event clickCount] == 1)
    {
        theViewer->getEventQueue()->mouseButtonPress(converted_point.x, converted_point.y, 3);
    }
    else
    {
        theViewer->getEventQueue()->mouseDoubleButtonPress(converted_point.x, converted_point.y, 3);
    }
    [self setNeedsDisplay:YES];
}


- (void) doRightMouseButtonUp:(NSEvent*)the_event
{
    // We must convert the mouse event locations from the window coordinate system to the
    // local view coordinate system.
    NSPoint the_point = [the_event locationInWindow];
    NSPoint converted_point = [self convertPoint:the_point fromView:nil];
    
    theViewer->getEventQueue()->mouseButtonRelease(converted_point.x, converted_point.y, 3);
    [self setNeedsDisplay:YES];
}

- (void) doMiddleMouseButtonDown:(NSEvent*)the_event
{
    // We must convert the mouse event locations from the window coordinate system to the
    // local view coordinate system.
    NSPoint the_point = [the_event locationInWindow];
    NSPoint converted_point = [self convertPoint:the_point fromView:nil];
    
    if([the_event clickCount] == 1)
    {
        theViewer->getEventQueue()->mouseButtonPress(converted_point.x, converted_point.y, 2);
    }
    else
    {
        theViewer->getEventQueue()->mouseDoubleButtonPress(converted_point.x, converted_point.y, 2);
    }
    [self setNeedsDisplay:YES];
}

- (void) doExtraMouseButtonDown:(NSEvent*)the_event buttonNumber:(int)button_number
{
    // We must convert the mouse event locations from the window coordinate system to the
    // local view coordinate system.
    NSPoint the_point = [the_event locationInWindow];
    NSPoint converted_point = [self convertPoint:the_point fromView:nil];

    if([the_event clickCount] == 1)
    {
        theViewer->getEventQueue()->mouseButtonPress(converted_point.x, converted_point.y, button_number+1);
    }
    else
    {
        theViewer->getEventQueue()->mouseDoubleButtonPress(converted_point.x, converted_point.y, button_number+1);
    }
    [self setNeedsDisplay:YES];
}


- (void) doMiddleMouseButtonUp:(NSEvent*)the_event
{
    // We must convert the mouse event locations from the window coordinate system to the
    // local view coordinate system.    NSPoint the_point = [the_event locationInWindow];
     NSPoint the_point = [the_event locationInWindow];
    NSPoint converted_point = [self convertPoint:the_point fromView:nil];
    theViewer->getEventQueue()->mouseButtonRelease(converted_point.x, converted_point.y, 2);
    [self setNeedsDisplay:YES];
}

- (void) doExtraMouseButtonUp:(NSEvent*)the_event buttonNumber:(int)button_number
{
    // We must convert the mouse event locations from the window coordinate system to the
    // local view coordinate system.    NSPoint the_point = [the_event locationInWindow];
    NSPoint the_point = [the_event locationInWindow];
    NSPoint converted_point = [self convertPoint:the_point fromView:nil];
    theViewer->getEventQueue()->mouseButtonRelease(converted_point.x, converted_point.y, button_number+1);
    
    [self setNeedsDisplay:YES];
}

// This is a job for Mighty Mouse!
// For the most fluid experience turn on 360 degree mode availble in 10.4.8+.
// With your Mighty Mouse plugged in, 
// open 'Keyboard & Mouse' in 'System Preferences'. 
// Select the 'Mouse' tab.
// Under 'Scrolling Options' select '360 degree'. 
// That should improve diagonal scrolling.
// You should also be able to use 'two-finger scrolling' on newer laptops.
- (void) scrollWheel:(NSEvent*)the_event
{
    // Unfortunately, it turns out mouseScroll2D doesn't actually do anything.
    // The camera manipulators don't seem to implement any code that utilize the scroll values.
    // This this call does nothing.
//    theViewer->getEventQueue()->mouseScroll2D([the_event deltaX], [the_event deltaY]);

    // With the absense of a useful mouseScroll2D API, we can manually simulate the desired effect.
    NSPoint the_point = [the_event locationInWindow];
    NSPoint converted_point = [self convertPoint:the_point fromView:nil];
    theViewer->getEventQueue()->mouseButtonPress(converted_point.x, converted_point.y, 1);
    theViewer->getEventQueue()->mouseMotion(converted_point.x + -[the_event deltaX], converted_point.y + [the_event deltaY]);
    theViewer->getEventQueue()->mouseButtonRelease(converted_point.x + -[the_event deltaX], converted_point.y + [the_event deltaY], 1);

    [self setNeedsDisplay:YES];
}

////////////////////////////////////////////////////////////////////////
/////////////////////////// End Mouse Stuff ////////////////////////////
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
/////////////////////////// Keyboard Stuff /////////////////////////////
////////////////////////////////////////////////////////////////////////
// Needed to accept keyboard events
- (BOOL) acceptsFirstResponder
{
    return YES;
}

- (void) keyDown:(NSEvent*)the_event
{
    // Do you want characters or charactersIgnoringModifiers?
    NSString* event_characters = [the_event characters];
//    NSString* event_characters = [the_event charactersIgnoringModifiers];

    unichar unicode_character = [event_characters characterAtIndex:0];
//    NSLog(@"unicode_character: %d", unicode_character);
    theViewer->getEventQueue()->keyPress(static_cast<osgGA::GUIEventAdapter::KeySymbol>(unicode_character));

    [self setNeedsDisplay:YES];
}

- (void) keyUp:(NSEvent*)the_event
{
    // Do you want characters or charactersIgnoringModifiers?
    NSString* event_characters = [the_event characters];
//    NSString* event_characters = [the_event charactersIgnoringModifiers];
    unichar unicode_character = [event_characters characterAtIndex:0];
    theViewer->getEventQueue()->keyRelease(static_cast<osgGA::GUIEventAdapter::KeySymbol>(unicode_character));
    [self setNeedsDisplay:YES];
}

////////////////////////////////////////////////////////////////////////
/////////////////////////// End Keyboard Stuff /////////////////////////
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
/////////////////////////// View and Draw Stuff ////////////////////////
////////////////////////////////////////////////////////////////////////

// This method is periodically called by my timer.
- (void) animationCallback
{
    // Simply notify Cocoa that a drawRect needs to take place.
    // Potential optimization is to query the OSG stuff to find out if a redraw is actually necessary.
    [self setNeedsDisplay:YES];
}

// This is an optional optimization. This states you don't have a transparent view/window.
// Obviously don't use this or set it to NO if you intend for your view to be see-through.
- (BOOL) isOpaque
{
    return YES;
}

// Resolution Independent UI is coming... (Tiger's Quartz Debug already has the tool.)
// We must think in 'point sizes', not pixel sizes, so a conversion is needed for OpenGL.
- (void) resizeViewport
{
    NSSize size_in_points = [self bounds].size;
    // This coordinate system conversion seems to make things work with Quartz Debug.
    NSSize size_in_window_coordinates = [self convertSize:size_in_points toView:nil];
    theViewer->getEventQueue()->windowResize(0, 0, size_in_window_coordinates.width, size_in_window_coordinates.height);
    graphicsWindow->resized(0, 0, size_in_window_coordinates.width, size_in_window_coordinates.height);
}

// For window resize
- (void) reshape
{
    [super reshape];
    [self resizeViewport];
}

// This is the code that actually draws.
// Remember you shouldn't call drawRect: directly and should use setNeedsDisplay:YES
// This is so the operating system can optimize when a draw is actually needed.
// (e.g. No sense drawing when the application is hidden.)
- (void) drawRect:(NSRect)the_rect
{
    if([[NSGraphicsContext currentContext] isDrawingToScreen])
    {
        [[self openGLContext] makeCurrentContext];
        theViewer->frame();
        [[self openGLContext] flushBuffer];
    }
    else // This is usually the print case
    {
        [[self openGLContext] makeCurrentContext];

        // FIXME: We should be computing a size that fits best to the paper target
        NSSize size_in_points = [self bounds].size;
        NSSize size_in_window_coordinates = [self convertSize:size_in_points toView:nil];
        NSBitmapImageRep * bitmap_image_rep = [self renderOpenGLSceneToFramebufferAsFormat:GL_RGB viewWidth:size_in_window_coordinates.width viewHeight:size_in_window_coordinates.height];
        
        NSImage* ns_image = [self imageFromBitmapImageRep:bitmap_image_rep];

        if(ns_image)
        {
            NSSize image_size = [ns_image size];
            [ns_image drawAtPoint:NSMakePoint(0.0, 0.0) 
                    fromRect: NSMakeRect(0.0, 0.0, image_size.width, image_size.height)
//                   operation: NSCompositeSourceOver
                   operation: NSCompositeCopy
                    fraction: 1.0];     
        }
        else
        {
            NSLog(@"Image not valid");
        }
    }
}


/* Optional render to framebuffer stuff below. The code renders offscreen to assist in screen capture stuff.
 * This can be useful for things like the Dock minimization picture, drag-and-drop dropImage, copy and paste,
 * and printing.
 */

// Convenience version. Will use the current view's bounds and produce an RGB image with the current clear color.
- (NSBitmapImageRep*) renderOpenGLSceneToFramebuffer
{
    NSSize size_in_points = [self bounds].size;
    NSSize size_in_window_coordinates = [self convertSize:size_in_points toView:nil];
    const osg::Vec4& clear_color = theViewer->getCamera()->getClearColor();

    return [self renderOpenGLSceneToFramebufferAsFormat:GL_RGB viewWidth:size_in_window_coordinates.width viewHeight:size_in_window_coordinates.height clearColorRed:clear_color[0] clearColorGreen:clear_color[1] clearColorBlue:clear_color[2] clearColorAlpha:clear_color[3]];
}

// Convenience version. Allows you to specify the view and height and format, but uses the current the current clear color.
- (NSBitmapImageRep*) renderOpenGLSceneToFramebufferAsFormat:(int)gl_format viewWidth:(float)view_width viewHeight:(float)view_height
{
    const osg::Vec4& clear_color = theViewer->getCamera()->getClearColor();

    return [self renderOpenGLSceneToFramebufferAsFormat:gl_format viewWidth:view_width viewHeight:view_height clearColorRed:clear_color[0] clearColorGreen:clear_color[1] clearColorBlue:clear_color[2] clearColorAlpha:clear_color[3]];
}

// Renders to an offscreen buffer and returns a copy of the data to an NSBitmapImageRep.
// Allows you to specify the gl_format, width and height, and the glClearColor
// gl_format is only GL_RGB or GLRGBA.
- (NSBitmapImageRep*) renderOpenGLSceneToFramebufferAsFormat:(int)gl_format viewWidth:(float)view_width viewHeight:(float)view_height clearColorRed:(float)clear_red clearColorGreen:(float)clear_green clearColorBlue:(float)clear_blue clearColorAlpha:(float)clear_alpha
{
    // Round values and bring to closest integer.
    int viewport_width = (int)(view_width + 0.5f);
    int viewport_height = (int)(view_height + 0.5f);
    
    NSBitmapImageRep* ns_image_rep;
    osg::ref_ptr<osg::Image> osg_image = new osg::Image;
    
    if(GL_RGBA == gl_format)
    {
        // Introduced in 10.4, but gives much better looking results if you utilize transparency
        if([NSBitmapImageRep instancesRespondToSelector:@selector(initWithBitmapDataPlanes:pixelsWide:pixelsHigh:bitsPerSample:samplesPerPixel:hasAlpha:isPlanar:colorSpaceName:bitmapFormat:bytesPerRow:bitsPerPixel:)])
        {
            ns_image_rep = [[[NSBitmapImageRep alloc] initWithBitmapDataPlanes:NULL
                                                  pixelsWide:viewport_width
                                                  pixelsHigh:viewport_height
                                               bitsPerSample:8
                                             samplesPerPixel:4
                                                    hasAlpha:YES
                                                    isPlanar:NO
                                              colorSpaceName:NSCalibratedRGBColorSpace
                                                bitmapFormat:NSAlphaNonpremultipliedBitmapFormat // 10.4+, gives much better looking results if you utilize transparency
                                                 bytesPerRow:osg::Image::computeRowWidthInBytes(viewport_width, GL_RGBA, GL_UNSIGNED_BYTE, 1)
                                                bitsPerPixel:32]
                    autorelease];
        }
        else // fallback for 10.0 to 10.3
        {
            ns_image_rep = [[[NSBitmapImageRep alloc] initWithBitmapDataPlanes:NULL
                                                  pixelsWide:viewport_width
                                                  pixelsHigh:viewport_height
                                               bitsPerSample:8
                                             samplesPerPixel:4
                                                    hasAlpha:YES
                                                    isPlanar:NO
                                              colorSpaceName:NSCalibratedRGBColorSpace
                                                // bitmapFormat:NSAlphaNonpremultipliedBitmapFormat // 10.4+, gives much better looking results if you utilize transparency
                                                 bytesPerRow:osg::Image::computeRowWidthInBytes(viewport_width, GL_RGBA, GL_UNSIGNED_BYTE, 1)
                                                bitsPerPixel:32]
                    autorelease];
        }
        // This is an optimization. Instead of creating data in both an osg::Image and NSBitmapImageRep,
        // Allocate just the memory in the NSBitmapImageRep and give the osg::Image a reference to the data.
        // I let NSBitmapImageRep control the memory because I think it will be easier to deal with 
        // memory management in the cases where it must interact with other Cocoa mechanisms like Drag-and-drop
        // where the memory persistence is less obvious. Make sure that you don't expect to use the osg::Image
        // outside the scope of this function because there is no telling when the data will be removed out
        // from under it by Cocoa since osg::Image will not retain.
        osg_image->setImage([ns_image_rep pixelsWide], [ns_image_rep pixelsHigh], 1,  GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, [ns_image_rep bitmapData], osg::Image::NO_DELETE, 1);
    }
    else if(GL_RGB == gl_format)
    {
        ns_image_rep = [[[NSBitmapImageRep alloc] initWithBitmapDataPlanes:NULL
                                                  pixelsWide:viewport_width
                                                  pixelsHigh:viewport_height
                                               bitsPerSample:8
                                             samplesPerPixel:3
                                                    hasAlpha:NO
                                                    isPlanar:NO
                                              colorSpaceName:NSCalibratedRGBColorSpace
                                                // bitmapFormat:(NSBitmapFormat)0 // 10.4+
                                                 bytesPerRow:osg::Image::computeRowWidthInBytes(viewport_width, GL_RGB, GL_UNSIGNED_BYTE, 1)
                                                bitsPerPixel:24]
                    autorelease];

        // This is an optimization. Instead of creating data in both an osg::Image and NSBitmapImageRep,
        // Allocate just the memory in the NSBitmapImageRep and give the osg::Image a reference to the data.
        // I let NSBitmapImageRep control the memory because I think it will be easier to deal with 
        // memory management in the cases where it must interact with other Cocoa mechanisms like Drag-and-drop
        // where the memory persistence is less obvious. Make sure that you don't expect to use the osg::Image
        // outside the scope of this function because there is no telling when the data will be removed out
        // from under it by Cocoa since osg::Image will not retain.
        osg_image->setImage([ns_image_rep pixelsWide], [ns_image_rep pixelsHigh], 1,  GL_RGB, GL_RGB, GL_UNSIGNED_BYTE, [ns_image_rep bitmapData], osg::Image::NO_DELETE, 1);
    }
    else
    {
        NSLog(@"Sorry, unsupported format in renderOpenGLSceneToFramebufferAsFormat");
        return nil;
    }

    // Can't find a way to query Viewer for the current values, so recompute current view size.
    NSSize original_size_in_points = [self bounds].size;
    NSSize original_size_in_window_coordinates = [self convertSize:original_size_in_points toView:nil];
//    theViewer->getEventQueue()->windowResize(0, 0, original_size_in_window_coordinates.width, original_size_in_window_coordinates.height);

    theViewer->getEventQueue()->windowResize(0, 0, viewport_width, viewport_height);
    graphicsWindow->resized(0, 0, viewport_width, viewport_height);

    /*
     * I want to use a Framebuffer Object because it seems to be the OpenGL sanctioned way of rendering offscreen.
     * Also, I want to try to decouple the image capture from the onscreen rendering. This is potentially useful
     * for two reasons:
     * 1) You may want to customize the image dimensions to best fit the situation (consider printing to a page to fit)
     * 2) You may want to customize the scene for the target (consider special output for a printer, or removed data for a thumbnail)
     * Unfortunately, I have hit two problems.
     * 1) osg::Camera (which seems to be the way to access Framebuffer Objects in OSG) doesn't seem to capture if it is the root node.
     * The workaround is to copy the camera attributes into another camera, and then add a second camera node into the scene.
     * I'm hoping OSG will simplify this in the future.
     * 2) I may have encountered a bug. Under some circumstances, the offscreen renderbuffer seems to get drawn into the onscreen view
     * when using a DragImage for drag-and-drop. I reproduced a non-OSG example, but learned I missed two important FBO calls which trigger gl errors.
     * So I'm wondering if OSG made the same mistake.
     * But the problem doesn't seem critical. It just looks bad.
     */
    //NSLog(@"Before camera glGetError: %s", gluErrorString(glGetError()));
    osg::Camera* root_camera = theViewer->getCamera();

    // I originally tried the clone() method and the copy construction, but it didn't work right,
    // so I manually copy the attributes.
    osg::Camera* the_camera = new osg::Camera;

    the_camera->setClearMask(root_camera->getClearMask());
    the_camera->setProjectionMatrix(root_camera->getProjectionMatrix());
    the_camera->setViewMatrix(root_camera->getViewMatrix());
    the_camera->setViewport(root_camera->getViewport());
    the_camera->setClearColor(
        osg::Vec4(
            clear_red,
            clear_green,
            clear_blue,
            clear_alpha
        )
    );

    // This must be ABSOLUTE_RF, and not a copy of the root camera because the transforms are additive.
    the_camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);

    // We need to insert the new (second) camera into the scene (below the root camera) and attach 
    // the scene data to the new camera.
    osg::ref_ptr<osg::Node> root_node = theViewer->getSceneData();

    the_camera->addChild(root_node.get());
    // Don't call (bypass) Viewer's setSceneData, but the underlying SceneView's setSceneData.
    // Otherwise, the camera position gets reset to the home position.
    theViewer->setSceneData(the_camera);

    // set the camera to render before the main camera.
    the_camera->setRenderOrder(osg::Camera::PRE_RENDER);

    // tell the camera to use OpenGL frame buffer object where supported.
    the_camera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);


    // attach the image so its copied on each frame.
    the_camera->attach(osg::Camera::COLOR_BUFFER, osg_image.get());


    //NSLog(@"Before frame(), glGetError: %s", gluErrorString(glGetError()));


    // Render the scene
    theViewer->frame();

    // Not sure if I really need this (seems to work without it), and if so, not sure if I need flush or finish
    glFlush();
//    glFinish();

    //NSLog(@"After flush(), glGetError: %s", gluErrorString(glGetError()));



    // The image is upside-down to Cocoa, so invert it.
    osg_image.get()->flipVertical();

    // Clean up everything I changed
//    the_camera->detach(osg::Camera::COLOR_BUFFER);
//    the_camera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER);
    // Don't call (bypass) Viewer's setSceneData, but the underlying SceneView's setSceneData.
    // Otherwise, the camera position gets reset to the home position.
    theViewer->setSceneData(root_node.get());
    theViewer->getEventQueue()->windowResize(0, 0, original_size_in_window_coordinates.width, original_size_in_window_coordinates.height);
    graphicsWindow->resized(0, 0, original_size_in_window_coordinates.width, original_size_in_window_coordinates.height);


    // Ugh. Because of the bug I mentioned, I'm losing the picture in the display when I print.
    [self setNeedsDisplay:YES];
    //NSLog(@"at return, glGetError: %s", gluErrorString(glGetError()));

    return ns_image_rep;
}

// Convenience method
- (NSImage*)imageFromBitmapImageRep:(NSBitmapImageRep*)bitmap_image_rep
{
    if(nil == bitmap_image_rep)
    {
        return nil;
    }
    NSImage* image = [[[NSImage alloc] initWithSize:[bitmap_image_rep size]] autorelease];
    [image addRepresentation:bitmap_image_rep];
    // This doesn't seem to work as I want it to. The image only gets flipped when rendered in a regular view.
    // It doesn't flip for the printer view. I must actually invert the pixels.
//    [image setFlipped:YES];
    return image;
}




////////////////////////////////////////////////////////////////////////
/////////////////////////// End View and Draw Stuff ////////////////////
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
/////////////////////////// For drag and drop //////////////////////////
////////////////////////////////////////////////////////////////////////
- (unsigned int) draggingEntered:(id <NSDraggingInfo>)the_sender
{
    if([the_sender draggingSource] != self)
    {
        NSPasteboard* paste_board = [the_sender draggingPasteboard];
        // I respond to filename types or URL types
        NSArray* supported_types = [NSArray arrayWithObjects:NSFilenamesPboardType, NSURLPboardType, nil];
        // If any of the supported types are being dragged in, activate the copy operation
        NSString* first_type = [paste_board availableTypeFromArray:supported_types];
        if(first_type != nil)
        {
            return NSDragOperationCopy;
        }
    }
    // Means we don't support this type
    return NSDragOperationNone;
}

// We're not using this method, but here it is as an example.
- (void) draggingExited:(id <NSDraggingInfo>)the_sender
{
}

- (BOOL) prepareForDragOperation:(id <NSDraggingInfo>)the_sender
{
    return YES;
}

- (BOOL) performDragOperation:(id <NSDraggingInfo>)the_sender
{
    NSPasteboard* paste_board = [the_sender draggingPasteboard];

 
    if([[paste_board types] containsObject:NSFilenamesPboardType])
    {
        NSArray* file_names = [paste_board propertyListForType:NSFilenamesPboardType];
//        int number_of_files = [file_names count];
        // Exercise for the reader: Try loading all files in the array
        NSString* single_file = [file_names objectAtIndex:0];
        osg::ref_ptr<osg::Node> loaded_model = osgDB::readNodeFile([single_file fileSystemRepresentation]);
        if(!loaded_model)
        {
            NSLog(@"File: %@ failed to load", single_file);
            return NO;
        }
        theViewer->setSceneData(loaded_model.get());
        return YES;
    }
    else if([[paste_board types] containsObject:NSURLPboardType])
    {
        NSURL* file_url = [NSURL URLFromPasteboard:paste_board];
        // See if the URL is valid file path
        if(![file_url isFileURL])
        {
            NSLog(@"URL: %@ needs to be a file for readNodeFile()", file_url);
            return NO;
        }
        NSString* file_path = [file_url path];
        osg::ref_ptr<osg::Node> loaded_model = osgDB::readNodeFile([file_path fileSystemRepresentation]);
        if(!loaded_model)
        {
            NSLog(@"URL: %@ failed to load, %@", file_url, file_path);
            return NO;
        }
        theViewer->setSceneData(loaded_model.get());
        return YES;
    }
    return NO;
}

// This method isn't really needed (I could move setNeedsDisplay up), but is here as an example
- (void) concludeDragOperation:(id <NSDraggingInfo>)the_sender
{
    [self setNeedsDisplay:YES];
}

////////////////////////////////////////////////////////////////////////
/////////////////////////// End of drag and drop (receiver) ////////////
////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////
/////////////////////////// For drag and drop and copy/paste (source) ////////////////
//////////////////////////////////////////////////////////////////////////////////////
- (IBAction) copy:(id)sender
{
    NSString* type = NSTIFFPboardType;
    NSData* image_data = [self contentsAsDataOfType:type];
        
    if(image_data)
    {
        NSPasteboard* general_pboard = [NSPasteboard generalPasteboard];
        [general_pboard declareTypes:[NSArray arrayWithObjects:type, nil] owner: nil];
        [general_pboard setData:image_data forType:type];
    }
}

- (NSData*) dataWithTIFFOfContentView
{
    [[self openGLContext] makeCurrentContext];
    NSBitmapImageRep * image = [self renderOpenGLSceneToFramebuffer];
    NSData* data = nil;

    if(image != nil)
    {
        data = [image TIFFRepresentation];
    }
    return data;
}

/* Returns a data object containing the current contents of the receiving window */
- (NSData*) contentsAsDataOfType:(NSString *)pboardType
{
    NSData * data = nil;
    if ([pboardType isEqualToString: NSTIFFPboardType] == YES)
    {
        data = [self dataWithTIFFOfContentView];
    }
    return data;
}


- (void) startDragAndDropAsSource:(NSEvent*)the_event
{
    NSPasteboard* drag_paste_board;
    NSImage* the_image;
    NSSize the_size;
    NSPoint the_point;

    NSSize size_in_points = [self bounds].size;
    NSSize size_in_window_coordinates = [self convertSize:size_in_points toView:nil];

    // Create the image that will be dragged
    NSString * type = NSTIFFPboardType;

    [[self openGLContext] makeCurrentContext];

    // I want two images. One to be rendered for the target, and one as the drag-image.
    // I want the drag-image to be translucent.
    // I think this is where render GL_COLOR_ATTACHMENTn (COLOR_BUFFERn?) would be handy.
    // But my hardware only returns 1 for glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS_EXT, maxbuffers);
    // So I won't bother and will just render twice.
    NSBitmapImageRep* bitmap_image_rep = [self renderOpenGLSceneToFramebufferAsFormat:GL_RGB viewWidth:size_in_window_coordinates.width viewHeight:size_in_window_coordinates.height];
    NSBitmapImageRep* bitmap_image_rep_transparent_copy = [self renderOpenGLSceneToFramebufferAsFormat:GL_RGBA viewWidth:size_in_window_coordinates.width viewHeight:size_in_window_coordinates.height];
//    NSBitmapImageRep* bitmap_image_rep = [self renderOpenGLSceneToFramebufferAsFormat:GL_RGBA viewWidth:size_in_window_coordinates.width viewHeight:size_in_window_coordinates.height clearColorRed:1.0f clearColorGreen:1.0f clearColorBlue:0.0f clearColorAlpha:0.4f];

//NSBitmapImageRep* bitmap_image_rep_transparent_copy = bitmap_image_rep;

    // 0x32 is an arbitrary number. Basically, I want something between 0 and 0xFF.
    Internal_SetAlpha(bitmap_image_rep_transparent_copy, 0x32);

    NSData* image_data = [bitmap_image_rep TIFFRepresentation];

    if(image_data)
    {
    
        drag_paste_board = [NSPasteboard pasteboardWithName:NSDragPboard];
        // is owner:self or nil? (Hillegass says self)
        [drag_paste_board declareTypes: [NSArray arrayWithObjects: type, nil] owner: self];
        [drag_paste_board setData:image_data forType: type];
        
        // create an image from the data
        the_image = [[NSImage alloc] initWithData:[bitmap_image_rep_transparent_copy TIFFRepresentation]];
        
        the_point = [self convertPoint:[the_event locationInWindow] fromView:nil];
        the_size = [the_image size];
        
        // shift the point to the center of the image
        the_point.x = the_point.x - the_size.width/2.0;
        the_point.y = the_point.y - the_size.height/2.0;

        // start drag
        [self dragImage:the_image
                     at:the_point
                 offset:NSMakeSize(0,0)
                  event:the_event
             pasteboard:drag_paste_board
                 source:self
              slideBack:YES];
              
        [the_image release];
    }
    else
    {
        NSLog(@"Error, failed to create image data");
    }
    
}

//////////////////////////////////////////////////////////////////////////////////////
/////////////////////////// For drag and drop and copy/paste (source) ////////////////
//////////////////////////////////////////////////////////////////////////////////////






////////////////////////////////////////////////////////////////////////
/////////////////////////// IBAction examples  /////////////////////////
////////////////////////////////////////////////////////////////////////

// Connect a button to this to stop and reset the position.
- (IBAction) resetPosition:(id)the_sender
{
    //    osgGA::MatrixManipulator* camera_manipulator = theViewer->getCameraManipulator();
    // This only resets the position
    //    camera_manipulator->home(0.0);
    
    // There is no external API from SimpleViewer that I can see that will stop movement.
    // So fake the 'spacebar' to stop things and reset.
    // (Changed in Viewer?)
    //    printf("I'am here"); 
    
    // Reset to start position
    theViewer->home();
    [self setNeedsDisplay:YES];
}

// Connect a NSColorWell to this to change color.
// A better way to do this is use Cocoa Bindings because it will automatically
// synchronize between your model and view, but since this demo lacks a model and controller
// aspect it wouldn't do much good.
- (IBAction) takeBackgroundColorFrom:(id)the_sender
{
    NSColor* the_color = [the_sender color];

    theViewer->getCamera()->setClearColor(
        osg::Vec4(
            [the_color redComponent],
            [the_color greenComponent],
            [the_color blueComponent],
            [the_color alphaComponent]
        )
    );
    [self setNeedsDisplay:YES];
}

- (IBAction) toggleFullScreen:(id)the_sender
{
    // I'm lazy and rather use the new 10.5 Cocoa Fullscreen API.
    // For now, no legacy support for fullscreen.
    // One of the cool things about Obj-C is dynamic/late binding.
    // We can compile and run this code on versions prior to 10.5.
    // At run-time, we check to see if these methods actually exist
    // and if they do, we message them. If not, we avoid them.
    if([self respondsToSelector:@selector(isInFullScreenMode)])
    {
        if([self isInFullScreenMode])
        {
            [self exitFullScreenModeWithOptions:nil];
        }
        else
        {
            [self enterFullScreenMode:[NSScreen mainScreen] withOptions:nil];
        }
    }
}

////////////////////////////////////////////////////////////////////////
/////////////////////////// End IBAction examples  /////////////////////
////////////////////////////////////////////////////////////////////////

@end
