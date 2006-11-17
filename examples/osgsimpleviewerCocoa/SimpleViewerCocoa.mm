//
//  SimpleViewerCocoa.mm
//  osgsimpleviewerCocoa
//
//  Created by Eric Wing on 11/12/06.
//  Copyright 2006. All rights reserved.
//
/* This class demonstrates how to subclass NSOpenGLView to integrate with the 
 * osgViewer::SimpleViewer.
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
 * Resolution Independent UI (maybe, not Leopard tested, only Tiger Quartz Debug)
 * Target-Action (for other widgets to invoke actions on this view)
 *
 * Things not demonstrated by this view or application example (but would be interesting):
 * Cocoa Bindings (highly recommended)
 * Core Data (works great with Cocoa Bindings)
 * Custom Interface Builder palette with Inspector configurable options
 * Shared OpenGLContexts
 * More PixelFormat options
 * Fullscreen mode
 * Low-level CGL access.
 * Delegates for your view
 * Multithreading
 * Drag-and-drop as a source (similar to the target code)
 * Copy, Cut, Paste (very similar to drag-and-drop)
 * Creating (updating) images of the window for the minimized view in the Dock
 * Printing support
 * Non-view stuff (Application Delegates, models, controllers)
 * Launching by double-clicking a model or drag-and-drop onto Application Icon (non-view)
 * Launching via commandline with parameters (non-view)
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
 * (e.g. [self initWithX:x_val yVal:y_val zVal:z_val].
 * (When I was first learning Objective-C, this made it easier for me to 
 * figure out which things * were paired.)
 */
#import "SimpleViewerCocoa.h"

#include <osgViewer/SimpleViewer>
#include <osgGA/TrackballManipulator>
// Needed to explicitly typecast keys to the OSG type
#include <osgGA/GUIEventAdapter>
// Needed to load models
#include <osgDB/ReadFile>

// Used so I can change the background (clear) color
#include <osgUtil/SceneView>
#include <osg/Vec4>

// For debugging
//#include <osg/Notify>

// osgText is used only to announce that you can use drag-and-drop to load models.
// osgViewer itself does not have a dependency on osgText.
#include <osg/ref_ptr>
#include <osgText/Text>
#include <osg/Geode>

@implementation SimpleViewerCocoa

// My simple pixel format definition
+ (NSOpenGLPixelFormat*) basicPixelFormat
{
	NSOpenGLPixelFormatAttribute pixel_attributes[] =
	{
		NSOpenGLPFAWindow,
		NSOpenGLPFADoubleBuffer,  // double buffered
		NSOpenGLPFADepthSize, (NSOpenGLPixelFormatAttribute)32, // depth buffer size in bits
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
		NSOpenGLPixelFormat* pixel_format = [SimpleViewerCocoa basicPixelFormat];
		[self setPixelFormat:pixel_format];
		[self commonInit];
	}
	return self;
}

// My custom methods to centralize common init stuff
- (void) commonInit
{
	isUsingCtrlClick = NO;
	isUsingOptionClick = NO;
	

	[self initOSGViewer];
	[self initAnimationTimer];
	
	// Register for Drag and Drop
	[self registerForDraggedTypes:[NSArray arrayWithObjects:NSFilenamesPboardType, NSURLPboardType, nil]];
}

// Allocate a SimpleViewer and do basic initialization. No assumption about having an
// a valid OpenGL context is made by this function.
- (void) initOSGViewer
{
//	osg::setNotifyLevel( osg::DEBUG_FP );
	simpleViewer = new osgViewer::SimpleViewer;
	// Cocoa follows the same coordinate convention as OpenGL. osgViewer's default is inverted.
	simpleViewer->getEventQueue()->getCurrentEventState()->setMouseYOrientation(osgGA::GUIEventAdapter::Y_INCREASING_UPWARDS);
	// Use a trackball manipulator...matches nicely with the Mighty Mouse Scrollball.
	simpleViewer->setCameraManipulator(new osgGA::TrackballManipulator);
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
    a_signature = [SimpleViewerCocoa instanceMethodSignatureForSelector:the_selector];
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
	delete simpleViewer;
	[super dealloc];
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
	long swap_interval = 1 ;
    [[self openGLContext] setValues:&swap_interval forParameter:NSOpenGLCPSwapInterval];
		 
	// This is also might be a good place to setup OpenGL state that OSG doesn't control.
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	// We need to tell the osgViewer what the viewport size is
	[self resizeViewport];


	// This is optional:
	// This is to setup some default text in the OpenGL view so the 
	// user knows that they should drag and drop a model into the view.
	osg::ref_ptr<osgText::Text> default_text = new osgText::Text;
    default_text->setAlignment(osgText::Text::CENTER_CENTER);
    default_text->setBackdropType(osgText::Text::OUTLINE);
    default_text->setAxisAlignment(osgText::Text::XZ_PLANE);
	default_text->setText("Drag-and-Drop\nyour .osg model here!");
	osg::ref_ptr<osg::Geode> the_geode = new osg::Geode;
	the_geode->addDrawable(default_text.get());
    simpleViewer->setSceneData(the_geode.get());

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
	
	simpleViewer->getEventQueue()->mouseMotion(converted_point.x, converted_point.y);
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
	if([self isUsingOptionClick] == YES)
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
	
	simpleViewer->getEventQueue()->mouseMotion(converted_point.x, converted_point.y);
	[self setNeedsDisplay:YES];
}

- (void) rightMouseUp:(NSEvent*)the_event
{
	[self doRightMouseButtonUp:the_event];
}

// "otherMouse" seems to capture middle button and any other buttons beyond (4th, etc).
- (void) otherMouseDown:(NSEvent*)the_event
{
	// We must convert the mouse event locations from the window coordinate system to the
	// local view coordinate system.
	NSPoint the_point = [the_event locationInWindow];
    NSPoint converted_point = [self convertPoint:the_point fromView:nil];
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
	
	simpleViewer->getEventQueue()->mouseMotion(converted_point.x, converted_point.y);
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
	}	[self doExtraMouseButtonUp:the_event buttonNumber:[the_event buttonNumber]];
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
		simpleViewer->getEventQueue()->mouseButtonPress(converted_point.x, converted_point.y, 1);
	}
	else
	{
		simpleViewer->getEventQueue()->mouseDoubleButtonPress(converted_point.x, converted_point.y, 1);
	}
	[self setNeedsDisplay:YES];
}

- (void) doLeftMouseButtonUp:(NSEvent*)the_event
{
	// We must convert the mouse event locations from the window coordinate system to the
	// local view coordinate system.
	NSPoint the_point = [the_event locationInWindow];
    NSPoint converted_point = [self convertPoint:the_point fromView:nil];
	
	simpleViewer->getEventQueue()->mouseButtonRelease(converted_point.x, converted_point.y, 1);
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
		simpleViewer->getEventQueue()->mouseButtonPress(converted_point.x, converted_point.y, 3);
	}
	else
	{
		simpleViewer->getEventQueue()->mouseDoubleButtonPress(converted_point.x, converted_point.y, 3);
	}
	[self setNeedsDisplay:YES];
}


- (void) doRightMouseButtonUp:(NSEvent*)the_event
{
	// We must convert the mouse event locations from the window coordinate system to the
	// local view coordinate system.
	NSPoint the_point = [the_event locationInWindow];
    NSPoint converted_point = [self convertPoint:the_point fromView:nil];
	
	simpleViewer->getEventQueue()->mouseButtonRelease(converted_point.x, converted_point.y, 3);
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
		simpleViewer->getEventQueue()->mouseButtonPress(converted_point.x, converted_point.y, 2);
	}
	else
	{
		simpleViewer->getEventQueue()->mouseDoubleButtonPress(converted_point.x, converted_point.y, 2);
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
		simpleViewer->getEventQueue()->mouseButtonPress(converted_point.x, converted_point.y, button_number+1);
	}
	else
	{
		simpleViewer->getEventQueue()->mouseDoubleButtonPress(converted_point.x, converted_point.y, button_number+1);
	}
	[self setNeedsDisplay:YES];
}


- (void) doMiddleMouseButtonUp:(NSEvent*)the_event
{
	// We must convert the mouse event locations from the window coordinate system to the
	// local view coordinate system.	NSPoint the_point = [the_event locationInWindow];
 	NSPoint the_point = [the_event locationInWindow];
	NSPoint converted_point = [self convertPoint:the_point fromView:nil];
	simpleViewer->getEventQueue()->mouseButtonRelease(converted_point.x, converted_point.y, 2);
	[self setNeedsDisplay:YES];
}

- (void) doExtraMouseButtonUp:(NSEvent*)the_event buttonNumber:(int)button_number
{
	// We must convert the mouse event locations from the window coordinate system to the
	// local view coordinate system.	NSPoint the_point = [the_event locationInWindow];
	NSPoint the_point = [the_event locationInWindow];
	NSPoint converted_point = [self convertPoint:the_point fromView:nil];
	simpleViewer->getEventQueue()->mouseButtonRelease(converted_point.x, converted_point.y, button_number+1);
	
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
//	simpleViewer->getEventQueue()->mouseScroll2D([the_event deltaX], [the_event deltaY]);

	// With the absense of a useful mouseScroll2D API, we can manually simulate the desired effect.
	NSPoint the_point = [the_event locationInWindow];
	NSPoint converted_point = [self convertPoint:the_point fromView:nil];
	simpleViewer->getEventQueue()->mouseButtonPress(converted_point.x, converted_point.y, 1);
	simpleViewer->getEventQueue()->mouseMotion(converted_point.x + -[the_event deltaX], converted_point.y + [the_event deltaY]);
	simpleViewer->getEventQueue()->mouseButtonRelease(converted_point.x + -[the_event deltaX], converted_point.y + [the_event deltaY], 1);

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
//	NSString* event_characters = [the_event charactersIgnoringModifiers];

	unichar unicode_character = [event_characters characterAtIndex:0];
//	NSLog(@"unicode_character: %d", unicode_character);
	simpleViewer->getEventQueue()->keyPress(static_cast<osgGA::GUIEventAdapter::KeySymbol>(unicode_character));

	[self setNeedsDisplay:YES];
}

- (void) keyUp:(NSEvent*)the_event
{
	// Do you want characters or charactersIgnoringModifiers?
	NSString* event_characters = [the_event characters];
//	NSString* event_characters = [the_event charactersIgnoringModifiers];
	unichar unicode_character = [event_characters characterAtIndex:0];
	simpleViewer->getEventQueue()->keyRelease(static_cast<osgGA::GUIEventAdapter::KeySymbol>(unicode_character));
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
	simpleViewer->getEventQueue()->windowResize(0, 0, size_in_window_coordinates.width, size_in_window_coordinates.height);
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
	[[self openGLContext] makeCurrentContext];
	simpleViewer->frame();
	[[self openGLContext] flushBuffer];
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
	    osg::ref_ptr<osg::Node> loaded_model = osgDB::readNodeFile([single_file UTF8String]);
		if(!loaded_model)
		{
			NSLog(@"File: %@ failed to load", single_file);
			return NO;
		}
		simpleViewer->setSceneData(loaded_model.get());
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
	    osg::ref_ptr<osg::Node> loaded_model = osgDB::readNodeFile([file_path UTF8String]);
		if(!loaded_model)
		{
			NSLog(@"URL: %@ failed to load, %@", file_url, file_path);
			return NO;
		}
		simpleViewer->setSceneData(loaded_model.get());
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
/////////////////////////// End of drag and drop ///////////////////////
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
/////////////////////////// IBAction examples  /////////////////////////
////////////////////////////////////////////////////////////////////////

// Connect a button to this to stop and reset the position.
- (IBAction) resetPosition:(id)the_sender
{
	//	osgGA::MatrixManipulator* camera_manipulator = simpleViewer->getCameraManipulator();
	// This only resets the position
	//	camera_manipulator->home(0.0);
	
	// There is no external API from SimpleViewer that I can see that will stop movement.
	// So fake the 'spacebar' to stop things and reset.
	simpleViewer->getEventQueue()->keyPress(static_cast<osgGA::GUIEventAdapter::KeySymbol>(0x20));
	[self setNeedsDisplay:YES];
}

// Connect a NSColorWell to this to change color.
// A better way to do this is use Cocoa Bindings because it will automatically
// synchronize between your model and view, but since this demo lacks a model and controller
// aspect it wouldn't do much good.
- (IBAction) takeBackgroundColorFrom:(id)the_sender
{
	NSColor* the_color = [the_sender color];
	osgUtil::SceneView* scene_view = simpleViewer->getSceneView();

	scene_view->setClearColor(
		osg::Vec4(
			[the_color redComponent],
			[the_color greenComponent],
			[the_color blueComponent],
			[the_color alphaComponent]
		)
	);
	[self setNeedsDisplay:YES];
}

////////////////////////////////////////////////////////////////////////
/////////////////////////// End IBAction examples  /////////////////////
////////////////////////////////////////////////////////////////////////

@end
