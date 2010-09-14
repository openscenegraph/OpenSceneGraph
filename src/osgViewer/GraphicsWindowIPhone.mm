

#include <iostream>
//#include <osgViewer/api/IPhone/PixelBufferIPhone>
#include <osgViewer/api/IPhone/GraphicsWindowIPhone>

#import <QuartzCore/QuartzCore.h>
#import <UIKit/UIKit.h>

#if OSG_GLES1_FEATURES
	#import <OpenGLES/ES1/glext.h>
#else
	#import <OpenGLES/ES2/glext.h>
#endif

#include "IPhoneUtils.h"


// ----------------------------------------------------------------------------------------------------------
// Basic container to trac the multi touch input state
// ----------------------------------------------------------------------------------------------------------
class InputState
{
public:
	InputState(void) {
		_touches = 0;
		_tapCount = 0;
		
		//stores first/previous touch coords 
		for(int i=0; i<4; i++)
		{
			_initTouch[i] = osg::Vec2(0.0f, 0.0f);
			_touchTravelVec[i] = osg::Vec2(0.0f, 0.0f);
		}
		
		//the distance between two touches
		_initTouchDistance = 0.0f;
		
		_touchZoom = 0.0f;
	}
	~InputState(void) {
	}
	
	enum InputType{
		
		TOUCH_BEGIN = 0,
		TOUCH_MOVED,
		TOUCH_END,
		GRAVITY_CHANGED
	};
	
	
	InputType _type;
	
	//touches occuring at this moment
	int _touches; 
	int _tapCount;
	
	//stores first/previous touch coords 
	osg::Vec2 _initTouch[4];
	
	//the distance between multi touches
	float _initTouchDistance;
	
	//direction/velocity vector of each touch
	osg::Vec2 _touchTravelVec[4];
	
	//pinch occuring
	float _touchZoom;
	
	osg::Vec3 _gravity;
	
	int _previousMouseButton;
	
	//convert the touch state into a mouse button states are
	//1 touch only, left mouse
	//2 touches with no zoom/pinch, middle mouse
	//2 tocuhes zooming/pinching, right mouse
	int TouchStateToMouseButton(){
		//if pinch is above the threhold
		//if(abs(_touchZoom) > 5)
		//{return 3;}//return right mouse
		
		//otherwise however many touches
		switch(_touches){
			case 1:
				return 1; //left mouse
			case 2:
				return 2; //middle mouse
			case 3:
				return 3; //middle mouse
			default:
				return -1;
		}
	}
				
	
};


#pragma mark GraphicsWindowIPhoneWindow

// ----------------------------------------------------------------------------------------------------------
// GraphicsWindowIPhoneWindow, implements canBecomeKeyWindow + canBecomeMainWindow
// ----------------------------------------------------------------------------------------------------------

@interface GraphicsWindowIPhoneWindow : UIWindow
{
}

- (BOOL) canBecomeKeyWindow;
- (BOOL) canBecomeMainWindow;

@end

@implementation GraphicsWindowIPhoneWindow


- (BOOL) canBecomeKeyWindow
{
    return YES;
}

- (BOOL) canBecomeMainWindow
{
    return YES;
}

@end

#pragma mark GraphicsWindowIPhoneGLView

// ----------------------------------------------------------------------------------------------------------
// GraphicsWindowIPhoneGLView
// custom UIView-class handling creation and display of frame/render buffers plus receives touch input
// ----------------------------------------------------------------------------------------------------------

@interface GraphicsWindowIPhoneGLView : UIView
{
    @private
        osgViewer::GraphicsWindowIPhone* _win;
		EAGLContext* _context;
        unsigned int _cachedModifierFlags;
	
		/* The pixel dimensions of the backbuffer */
		GLint _backingWidth;
		GLint _backingHeight;
	
		//the pixel buffers for the video
		/* OpenGL names for the renderbuffer and framebuffers used to render to this view */
		GLuint _viewRenderbuffer, _viewFramebuffer;
		
		/* OpenGL name for the depth buffer that is attached to viewFramebuffer, if it exists (0 if it does not exist) */
		GLuint _depthRenderbuffer;
	
		/* OpenGL name for the stencil buffer that is attached to viewFramebuffer, if it exists (0 if it does not exist) */
		GLuint _stencilBuffer;
	
		InputState _inputState;
}

- (void)setGraphicsWindow: (osgViewer::GraphicsWindowIPhone*) win;
- (void)setOpenGLContext: (EAGLContext*) context;

- (BOOL)createFramebuffer;
- (void)destroyFramebuffer;
- (void)swapBuffers;
- (void)bindFrameBuffer;

- (BOOL)acceptsFirstResponder;
- (BOOL)becomeFirstResponder;
- (BOOL)resignFirstResponder;

@end

@implementation GraphicsWindowIPhoneGLView 


-(void) setGraphicsWindow: (osgViewer::GraphicsWindowIPhone*) win
{
    _win = win;
}

-(void) setOpenGLContext: (EAGLContext*) context
{
    _context = context;
}

// You must implement this method
+ (Class)layerClass {
    return [CAEAGLLayer class];
}

//
//Called when the view is created using a frame for dimensions
//
- (id)initWithFrame:(CGRect)frame : (osgViewer::GraphicsWindowIPhone*)win{
	
	_win = win;

    if ((self = [super initWithFrame:frame])) {
        // Get the layer
        CAEAGLLayer *eaglLayer = (CAEAGLLayer *)self.layer;
        
        eaglLayer.opaque = YES;
		if(_win->getTraits()->alpha > 0)
		{
			//create layer with alpha channel RGBA8
			eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
											[NSNumber numberWithBool:NO], kEAGLDrawablePropertyRetainedBacking, kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat, nil];
		}else{
			//else no alpha, iphone uses RBG565
			eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
											[NSNumber numberWithBool:NO], kEAGLDrawablePropertyRetainedBacking, kEAGLColorFormatRGB565, kEAGLDrawablePropertyColorFormat, nil];

		}
    }
    return self;
}

- (void)layoutSubviews {
    [EAGLContext setCurrentContext:_context];
    [self destroyFramebuffer];
    [self createFramebuffer];
}


- (BOOL)createFramebuffer {
    
    glGenFramebuffersOES(1, &_viewFramebuffer);
    glGenRenderbuffersOES(1, &_viewRenderbuffer);
    
    glBindFramebufferOES(GL_FRAMEBUFFER_OES, _viewFramebuffer);
    glBindRenderbufferOES(GL_RENDERBUFFER_OES, _viewRenderbuffer);
    [_context renderbufferStorage:GL_RENDERBUFFER_OES fromDrawable:(CAEAGLLayer*)self.layer];
    glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_RENDERBUFFER_OES, _viewRenderbuffer);
    
    glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_WIDTH_OES, &_backingWidth);
    glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_HEIGHT_OES, &_backingHeight);

	//add depth if requested
	if(_win->getTraits()->depth > 0) {
        glGenRenderbuffersOES(1, &_depthRenderbuffer);
        glBindRenderbufferOES(GL_RENDERBUFFER_OES, _depthRenderbuffer);
		if(_win->getTraits()->depth == 16)
		{
			glRenderbufferStorageOES(GL_RENDERBUFFER_OES, GL_DEPTH_COMPONENT16_OES, _backingWidth, _backingHeight);
		}else if(_win->getTraits()->depth == 24){
			glRenderbufferStorageOES(GL_RENDERBUFFER_OES, GL_DEPTH_COMPONENT24_OES, _backingWidth, _backingHeight);
		}
        glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_DEPTH_ATTACHMENT_OES, GL_RENDERBUFFER_OES, _depthRenderbuffer);
    }
	
	//add stencil if requested
	if(_win->getTraits()->stencil > 0) {
		glGenRenderbuffersOES(1, &_stencilBuffer);
		glBindRenderbufferOES(GL_RENDERBUFFER_OES, _stencilBuffer);
		glRenderbufferStorageOES(GL_RENDERBUFFER_OES, GL_STENCIL_INDEX8_OES, _backingWidth, _backingHeight);
		glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_STENCIL_ATTACHMENT_OES, GL_RENDERBUFFER_OES, _stencilBuffer);
    }	
    
    if(glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES) != GL_FRAMEBUFFER_COMPLETE_OES) {
        NSLog(@"failed to make complete framebuffer object %x", glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES));
        return NO;
    }
    
    return YES;
}


- (void)destroyFramebuffer {
    
    glDeleteFramebuffersOES(1, &_viewFramebuffer);
    _viewFramebuffer = 0;
    glDeleteRenderbuffersOES(1, &_viewRenderbuffer);
    _viewRenderbuffer = 0;
    
    if(_depthRenderbuffer) {
        glDeleteRenderbuffersOES(1, &_depthRenderbuffer);
        _depthRenderbuffer = 0;
    }
	
	if(_stencilBuffer) {
		glDeleteFramebuffersOES(1, &_stencilBuffer);
		_stencilBuffer = 0;
	}
}

//
//Swap the view and render buffers
//
- (void)swapBuffers {

	//swap buffers (sort of i think?)
    glBindRenderbufferOES(GL_RENDERBUFFER_OES, _viewRenderbuffer);
	//display render in context
    [_context presentRenderbuffer:GL_RENDERBUFFER_OES];
	
	//re bind the frame buffer for next frames renders
	glBindFramebufferOES(GL_FRAMEBUFFER_OES, _viewFramebuffer);
}

//
//bind view buffer as current for new render pass
//
- (void)bindFrameBuffer {

	//bind the frame buffer
	glBindFramebufferOES(GL_FRAMEBUFFER_OES, _viewFramebuffer);
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

//
//Touch input callbacks
//
- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event {
	
	NSSet *allTouches = [event allTouches];
    
	
	_inputState._type = InputState::TOUCH_BEGIN;
	_inputState._touches = [allTouches count];
	
	for(int i=0; i<[allTouches count]; i++)
	{
		
		UITouch *touch = [[allTouches allObjects] objectAtIndex:i];
		CGPoint pos = [touch locationInView:touch.view];
		_inputState._initTouch[i].x() = pos.x;
		_inputState._initTouch[i].y() = pos.y;
	}
	
	//press then relevant button
	int newButton = _inputState.TouchStateToMouseButton();
	_win->getEventQueue()->mouseButtonPress(_inputState._initTouch[0].x(),  _inputState._initTouch[0].y(), newButton);
	
	//store previous mouseButton
	_inputState._previousMouseButton = newButton;

	
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event {
    
    NSSet *allTouches = [event allTouches];
	
	_inputState._touchZoom = 0.0f;
	
	_inputState._type = InputState::TOUCH_BEGIN;
	_inputState._touches = [allTouches count];
	
	for(int i=0; i<[allTouches count]; i++)
	{
		
		UITouch *touch = [[allTouches allObjects] objectAtIndex:i];
		CGPoint pos = [touch locationInView:touch.view];
		osg::Vec2 cPos = osg::Vec2(pos.x, pos.y);
		 _inputState._touchTravelVec[i] = cPos - _inputState._initTouch[i];
		_inputState._initTouch[i] = cPos;
	}

	
	//during touchesMoved we need to check for changes in tocuh count and release/press the relevent mouse buttons
	int newButton = _inputState.TouchStateToMouseButton();
	//compare to previous button
	if(newButton != _inputState._previousMouseButton)
	{
		//unpress previous
		_win->getEventQueue()->mouseButtonRelease(_inputState._initTouch[0].x(),  _inputState._initTouch[0].y(), _inputState._previousMouseButton);
		//press new
		_win->getEventQueue()->mouseButtonPress(_inputState._initTouch[0].x(),  _inputState._initTouch[0].y(), newButton);
	}else{
		
		_win->getEventQueue()->mouseMotion(_inputState._initTouch[0].x(), _inputState._initTouch[0].y());
	}
	
	//store previous mouseButton
	_inputState._previousMouseButton = newButton;
	
	_inputState._type = InputState::TOUCH_MOVED;
		
	

}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event

{
	
    UITouch *touch = [[event allTouches] anyObject];
	
	//unpress previous
	_win->getEventQueue()->mouseButtonRelease(_inputState._initTouch[0].x(),  _inputState._initTouch[0].y(), _inputState._previousMouseButton);


	_inputState._type = InputState::TOUCH_END;
	
    _inputState._tapCount = [touch tapCount];
	
	//make double tap do space bar so we can get back to home position
	if( _inputState._tapCount == 2)
	{
		_win->getEventQueue()->keyPress( osgGA::GUIEventAdapter::KEY_Space, [event timestamp]);
		_win->getEventQueue()->keyRelease( osgGA::GUIEventAdapter::KEY_Space, [event timestamp]);
	}
	
	_inputState._initTouchDistance = -1.0f;
}


@end


#pragma mark CocoaWindowAdapter


using namespace osgIPhone; 
namespace osgViewer {

	
	
#pragma mark GraphicsWindowIPhone



// ----------------------------------------------------------------------------------------------------------
// init
// ----------------------------------------------------------------------------------------------------------

void GraphicsWindowIPhone::init()
{
    if (_initialized) return;

    _ownsWindow = false;
    _context = NULL;
    _window = NULL;
    _updateContext = false;
    _valid = _initialized = true;
}


// ----------------------------------------------------------------------------------------------------------
// realizeImplementation, creates the window + context
// ----------------------------------------------------------------------------------------------------------

bool GraphicsWindowIPhone::realizeImplementation()
{
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
    
    unsigned int style=0;//(NSBorderlessWindowMask);
    if (_traits->windowDecoration) {
			[[UIApplication sharedApplication] setStatusBarHidden:NO animated:NO];
    }else{
			[[UIApplication sharedApplication] setStatusBarHidden:YES animated:NO];
	}
        
    IPhoneWindowingSystemInterface* wsi = dynamic_cast<IPhoneWindowingSystemInterface*>(osg::GraphicsContext::getWindowingSystemInterface());
    int screenLeft(0), screenTop(0);
    if (wsi) {
        wsi->getScreenTopLeft((*_traits), screenLeft, screenTop);
    }
    
    CGRect rect =  [[UIScreen mainScreen] bounds];// = //CGRectMake((CGFloat)(_traits->x + screenLeft), (CGFloat)(_traits->y + screenTop), (CGFloat)(_traits->width), (CGFloat)(_traits->height));
	rect.origin.x = _traits->x; rect.origin.y = _traits->y;
	rect.size.width = _traits->width; rect.size.height = _traits->height;
	
    _ownsWindow = true;
    
    // see if an existing inherited window was passed in
    WindowData* windowData = _traits->inheritedWindowData ? dynamic_cast<WindowData*>(_traits->inheritedWindowData.get()) : NULL;
    if (windowData) 
    {
        if (windowData->_window)
		{
            _ownsWindow = false;        
			_window = windowData->_window;
		}
    } 
    

    osg::notify(osg::DEBUG_INFO) << "GraphicsWindowIPhone::realizeImplementation / ownsWindow: " << _ownsWindow << std::endl;

    if (_ownsWindow) 
    {
		//create the IPhone window object
        _window = [[GraphicsWindowIPhoneWindow alloc] initWithFrame: rect];// styleMask: style backing: NSBackingStoreBuffered defer: NO];
        
        if (!_window) {
            osg::notify(osg::WARN) << "GraphicsWindowIPhone::realizeImplementation :: could not create window" << std::endl;
            return false;
        }
    } 
            
	//create the desired OpenGLES context type
#if OSG_GLES1_FEATURES
	_context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1];
#elif OSG_GLES2_FEATURES
	_context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
#endif
	
	if (!_context || ![EAGLContext setCurrentContext:_context]) {
		osg::notify(osg::WARN) << "GraphicsWindowIPhone::realizeImplementation :: could not create graphics context" << std::endl;
		return false;
	}

	//create the view to display our context in our window
    GraphicsWindowIPhoneGLView* theView = [[ GraphicsWindowIPhoneGLView alloc ] initWithFrame:[ _window frame ] : this ];
    //[theView setAutoresizingMask:  (EAGLViewWidthSizable | EAGLViewHeightSizable) ];
    [theView setGraphicsWindow: this];
    [theView setOpenGLContext:_context];
    _view = theView;
    osg::notify(osg::DEBUG_INFO) << "GraphicsWindowIPhone::realizeImplementation / view: " << theView << std::endl;

	
	// Attach view to window
	[_window addSubview: _view];
	[theView release];
	
	//if we own the window also make it visible
    if (_ownsWindow) {
		//show window
		[_window makeKeyAndVisible];

    }

    [pool release];
    
    // iphones origin is top/left:
    getEventQueue()->getCurrentEventState()->setMouseYOrientation(osgGA::GUIEventAdapter::Y_INCREASING_DOWNWARDS);
    
    _valid = _initialized = _realized = true;
    return _valid;
}




// ----------------------------------------------------------------------------------------------------------
// closeImplementation
// ----------------------------------------------------------------------------------------------------------
void GraphicsWindowIPhone::closeImplementation()
{
    _valid = false;
    _realized = false;
    
    
    if (_view) {
        [_view setGraphicsWindow: NULL];
    }
        
    if (_window) {  
		[_window release];
		//[glView release];
	}

    
    _window = NULL;
    _view = NULL;    
}


// ----------------------------------------------------------------------------------------------------------
// makeCurrentImplementation
// ----------------------------------------------------------------------------------------------------------

bool GraphicsWindowIPhone:: makeCurrentImplementation()
{
    if (_updateContext)
    {
        //[_context update];
        _updateContext = false; 
    }
    
	//bind the context
    [EAGLContext setCurrentContext:_context];
	
	//i think we also want to bind the frame buffer here
	//[_view bindFrameBuffer];

    return true;
}


// ----------------------------------------------------------------------------------------------------------
// releaseContextImplementation
// ----------------------------------------------------------------------------------------------------------

bool GraphicsWindowIPhone::releaseContextImplementation()
{
	if ([EAGLContext currentContext] == _context) {
        [EAGLContext setCurrentContext:nil];
    }
    return true;
}


// ----------------------------------------------------------------------------------------------------------
// swapBuffersImplementation
// ----------------------------------------------------------------------------------------------------------

void GraphicsWindowIPhone::swapBuffersImplementation()
{
    //[_context flushBuffer];
	[_view swapBuffers];
}



// ----------------------------------------------------------------------------------------------------------
// setWindowDecorationImplementation
//
// We will use this to toggle the status bar on IPhone, nearest thing to window decoration
// ----------------------------------------------------------------------------------------------------------

bool GraphicsWindowIPhone::setWindowDecorationImplementation(bool flag)
{
    if (!_realized || !_ownsWindow) return false;

	if (flag) {
		[[UIApplication sharedApplication] setStatusBarHidden:NO animated:NO];
    }else{
		[[UIApplication sharedApplication] setStatusBarHidden:YES animated:NO];
	}
	
    return true;
}


// ----------------------------------------------------------------------------------------------------------
// grabFocus
// ----------------------------------------------------------------------------------------------------------
void GraphicsWindowIPhone::grabFocus()
{
	//i think make key is the equivalent of focus on iphone 
	[_window makeKeyWindow];
}


// ----------------------------------------------------------------------------------------------------------
// grabFocusIfPointerInWindow
// ----------------------------------------------------------------------------------------------------------
void GraphicsWindowIPhone::grabFocusIfPointerInWindow()
{
    osg::notify(osg::INFO) << "GraphicsWindowIPhone :: grabFocusIfPointerInWindow not implemented yet " << std::endl;
}

// ----------------------------------------------------------------------------------------------------------
// raiseWindow
// Raise the window to the top.
// ----------------------------------------------------------------------------------------------------------
void GraphicsWindowIPhone::raiseWindow()
{
	[_window bringSubviewToFront:_view];
}

// ----------------------------------------------------------------------------------------------------------
// resizedImplementation
// ----------------------------------------------------------------------------------------------------------

void GraphicsWindowIPhone::resizedImplementation(int x, int y, int width, int height)
{
    GraphicsContext::resizedImplementation(x, y, width, height);
    
    _updateContext = true;
    
	getEventQueue()->windowResize(x,y,width, height, getEventQueue()->getTime());
}




// ----------------------------------------------------------------------------------------------------------
// setWindowRectangleImplementation
// ----------------------------------------------------------------------------------------------------------
bool GraphicsWindowIPhone::setWindowRectangleImplementation(int x, int y, int width, int height)
{
    osg::notify(osg::INFO) << "GraphicsWindowIPhone :: setWindowRectangleImplementation not implemented yet " << std::endl;
    if (!_ownsWindow)
        return false;
        
    NSAutoreleasePool* localPool = [[NSAutoreleasePool alloc] init];
        
    IPhoneWindowingSystemInterface* wsi = dynamic_cast<IPhoneWindowingSystemInterface*>(osg::GraphicsContext::getWindowingSystemInterface());
    int screenLeft(0), screenTop(0);
    if (wsi) {
        wsi->getScreenTopLeft((*_traits), screenLeft, screenTop);
    }

    CGRect rect = CGRectMake(x+screenLeft,y+screenTop,width, height);    
    [localPool release];
    
    return true;
}

	
void GraphicsWindowIPhone::checkEvents()
{
	
	
}

// ----------------------------------------------------------------------------------------------------------
// 
// ----------------------------------------------------------------------------------------------------------

void GraphicsWindowIPhone::adaptResize(int x, int y, int w, int h)
{

    osg::notify(osg::INFO) << "GraphicsWindowIPhone :: adaptResize not implemented yet " << std::endl;
    IPhoneWindowingSystemInterface* wsi = dynamic_cast<IPhoneWindowingSystemInterface*>(osg::GraphicsContext::getWindowingSystemInterface());
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


// ----------------------------------------------------------------------------------------------------------
// setWindowName
// ----------------------------------------------------------------------------------------------------------

void GraphicsWindowIPhone::setWindowName (const std::string & name)
{
    osg::notify(osg::INFO) << "GraphicsWindowIPhone :: setWindowName not implemented yet " << std::endl;
}


// ----------------------------------------------------------------------------------------------------------
// useCursor, no cursor on IPhone
// ----------------------------------------------------------------------------------------------------------

void GraphicsWindowIPhone::useCursor(bool cursorOn)
{
    osg::notify(osg::INFO) << "GraphicsWindowIPhone :: useCursor not implemented yet " << std::endl;
}


// ----------------------------------------------------------------------------------------------------------
// setCursor, no cursor on IPhone
// ----------------------------------------------------------------------------------------------------------

void GraphicsWindowIPhone::setCursor(MouseCursor mouseCursor)
{
    osg::notify(osg::INFO) << "GraphicsWindowIPhone :: setCursor not implemented yet " << std::endl;
}


// ----------------------------------------------------------------------------------------------------------
// setVSync, no vsync on IPhone
// ----------------------------------------------------------------------------------------------------------

void GraphicsWindowIPhone::setVSync(bool f) 
{
    osg::notify(osg::INFO) << "GraphicsWindowIPhone :: setVSync not implemented yet " << std::endl;
}


// ----------------------------------------------------------------------------------------------------------
// d'tor
// ----------------------------------------------------------------------------------------------------------

GraphicsWindowIPhone::~GraphicsWindowIPhone() 
{
	//[release _view];
	//[release _window]
    close();
}



class CocoaWindowingSystemInterface : public  IPhoneWindowingSystemInterface {
public:
    CocoaWindowingSystemInterface()
    :    IPhoneWindowingSystemInterface()
    {
        // register application event handler and AppleEventHandler to get quit-events:
       // static const EventTypeSpec menueventSpec = {kEventClassCommand, kEventCommandProcess};
       // OSErr status = InstallEventHandler(GetApplicationEventTarget(), NewEventHandlerUPP(ApplicationEventHandler), 1, &menueventSpec, 0, NULL);
       // status = AEInstallEventHandler( kCoreEventClass, kAEQuitApplication, NewAEEventHandlerUPP(QuitAppleEventHandler), 0, false);
    }
    
    virtual osg::GraphicsContext* createGraphicsContext(osg::GraphicsContext::Traits* traits) 
    {											//osg::GraphicsContext
        return createGraphicsContextImplementation<GraphicsWindowIPhone, GraphicsWindowIPhone>(traits);
    }
};

}//end namspace


RegisterWindowingSystemInterfaceProxy<osgViewer::CocoaWindowingSystemInterface> createWindowingSystemInterfaceProxy;


// declare C entry point for static compilation.
extern "C" void graphicswindow_IPhone(void)
{
    osg::GraphicsContext::setWindowingSystemInterface(new osgViewer::CocoaWindowingSystemInterface());
}
