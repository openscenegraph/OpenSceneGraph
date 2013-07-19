

#include <iostream>
#include <osgViewer/api/IOS/GraphicsWindowIOS>

#import <QuartzCore/QuartzCore.h>
#import <UIKit/UIKit.h>

#if OSG_GLES1_FEATURES
    #import <OpenGLES/ES1/glext.h>
#else
    #import <OpenGLES/ES2/glext.h>
    // in GLES2, the OES suffix if dropped from function names (from rti) 
    #define glGenFramebuffersOES glGenFramebuffers 
    #define glGenRenderbuffersOES glGenRenderbuffers 
    #define glBindFramebufferOES glBindFramebuffer 
    #define glBindRenderbufferOES glBindRenderbuffer 
    #define glFramebufferRenderbufferOES glFramebufferRenderbuffer 
    #define glGetRenderbufferParameterivOES glGetRenderbufferParameteriv 
    #define glRenderbufferStorageOES glRenderbufferStorage 
    #define glDeleteRenderbuffersOES glDeleteRenderbuffers 
    #define glDeleteFramebuffersOES glDeleteFramebuffers 
    #define glCheckFramebufferStatusOES glCheckFramebufferStatus 

    #define GL_FRAMEBUFFER_OES GL_FRAMEBUFFER 
    #define GL_RENDERBUFFER_OES GL_RENDERBUFFER 
    #define GL_RENDERBUFFER_WIDTH_OES GL_RENDERBUFFER_WIDTH 
    #define GL_RENDERBUFFER_HEIGHT_OES GL_RENDERBUFFER_HEIGHT 
    #define GL_COLOR_ATTACHMENT0_OES GL_COLOR_ATTACHMENT0 
    #define GL_DEPTH_ATTACHMENT_OES GL_DEPTH_ATTACHMENT 
    #define GL_DEPTH_COMPONENT16_OES GL_DEPTH_COMPONENT16 
    #define GL_STENCIL_INDEX8_OES GL_STENCIL_INDEX8 
    #define GL_FRAMEBUFFER_COMPLETE_OES GL_FRAMEBUFFER_COMPLETE 
    #define GL_STENCIL_ATTACHMENT_OES GL_STENCIL_ATTACHMENT 

    #define GL_RGB5_A1_OES GL_RGB5_A1
#endif 

#include "IOSUtils.h"




#pragma mark GraphicsWindowIOSWindow

// ----------------------------------------------------------------------------------------------------------
// GraphicsWindowIOSWindow, implements canBecomeKeyWindow + canBecomeMainWindow
// ----------------------------------------------------------------------------------------------------------

@interface GraphicsWindowIOSWindow : UIWindow
{
}

- (BOOL) canBecomeKeyWindow;
- (BOOL) canBecomeMainWindow;

@end

@implementation GraphicsWindowIOSWindow

//
//Implement dealloc 
//
- (void) dealloc
{
    [super dealloc];
}

- (BOOL) canBecomeKeyWindow
{
    return YES;
}

- (BOOL) canBecomeMainWindow
{
    return YES;
}

@end

#pragma mark GraphicsWindowIOSGLView

// ----------------------------------------------------------------------------------------------------------
// GraphicsWindowIOSGLView
// custom UIView-class handling creation and display of frame/render buffers plus receives touch input
// ----------------------------------------------------------------------------------------------------------

typedef std::map<void*, unsigned int> TouchPointsIdMapping;

@interface GraphicsWindowIOSGLView : UIView
{
    @private
        osgViewer::GraphicsWindowIOS* _win;
        EAGLContext* _context;
    
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
        
        // for multisampled antialiased rendering
        GLuint _msaaFramebuffer, _msaaRenderBuffer, _msaaDepthBuffer;
        
        TouchPointsIdMapping* _touchPointsIdMapping;
        unsigned int _lastTouchPointId;


    
}

- (void)setGraphicsWindow: (osgViewer::GraphicsWindowIOS*) win;
- (osgViewer::GraphicsWindowIOS*) getGraphicsWindow;
- (void)setOpenGLContext: (EAGLContext*) context;
- (void)updateDimensions;
- (BOOL)createFramebuffer;
- (void)destroyFramebuffer;
- (void)swapBuffers;
- (void)bindFrameBuffer;

- (BOOL)acceptsFirstResponder;
- (BOOL)becomeFirstResponder;
- (BOOL)resignFirstResponder;

- (osgGA::GUIEventAdapter::TouchPhase) convertTouchPhase: (UITouchPhase) phase;
- (osg::Vec2) convertPointToPixel: (osg::Vec2) point;
- (void) dealloc;
@end

@implementation GraphicsWindowIOSGLView 

- (osgGA::GUIEventAdapter::TouchPhase) convertTouchPhase: (UITouchPhase) phase 
{
    switch(phase) {
    
        case UITouchPhaseBegan:
            return osgGA::GUIEventAdapter::TOUCH_BEGAN;
            break;
        case UITouchPhaseMoved:
            return osgGA::GUIEventAdapter::TOUCH_MOVED;
            break;

        case UITouchPhaseStationary:
            return osgGA::GUIEventAdapter::TOUCH_STATIONERY;
            break;

        case UITouchPhaseEnded:
        case UITouchPhaseCancelled:
            return osgGA::GUIEventAdapter::TOUCH_ENDED;
            break;
    }
    
    return osgGA::GUIEventAdapter::TOUCH_ENDED;

}


- (unsigned int)computeTouchId: (UITouch*) touch 
{
    unsigned int result(0);
    
    if (!_touchPointsIdMapping) {
        _lastTouchPointId = 0;
        _touchPointsIdMapping = new TouchPointsIdMapping();
    }
    
    switch([touch phase])
    {
    
        case UITouchPhaseBegan:
            {
                TouchPointsIdMapping::iterator itr = _touchPointsIdMapping->find(touch);
                // std::cout << "new: " << touch << " num: " << _touchPointsIdMapping->size() << " found: " << (itr != _touchPointsIdMapping->end()) << std::endl;
                 
                if (itr == _touchPointsIdMapping->end()) 
                {
                    (*_touchPointsIdMapping)[touch] = result = _lastTouchPointId;
                    _lastTouchPointId++;
                    break;
                }
               
            }
            // missing "break" by intention!
        
        case UITouchPhaseMoved:
        case UITouchPhaseStationary:
            {
                result = (*_touchPointsIdMapping)[touch];
            }
            break;
       
        case UITouchPhaseEnded:
        case UITouchPhaseCancelled:
            {
                TouchPointsIdMapping::iterator itr = _touchPointsIdMapping->find(touch);
                // std::cout<< "remove: " << touch << " num: " << _touchPointsIdMapping->size() << " found: " << (itr != _touchPointsIdMapping->end()) << std::endl;
                
                if (itr != _touchPointsIdMapping->end()) {
                    result = itr->second;
                    _touchPointsIdMapping->erase(itr);
                }
                if(_touchPointsIdMapping->size() == 0) {
                    _lastTouchPointId = 0;
                }
                // std::cout<< "remove: " << touch << " num: " << _touchPointsIdMapping->size() << std::endl;
            }
            break;
            
        default:
            break;
    }
        
    return result;
}


- (osg::Vec2) convertPointToPixel: (osg::Vec2) point
{
    //get the views contentscale factor and multiply the point by it
    float scale = 1.0f;
    
#if defined(__IPHONE_4_0) && (__IPHONE_OS_VERSION_MIN_REQUIRED >= __IPHONE_4_0)
    scale = self.contentScaleFactor;
#endif
    return osg::Vec2(point.x()*scale, point.y()*scale);
    
}

-(void) setGraphicsWindow: (osgViewer::GraphicsWindowIOS*) win
{
    _win = win;
    _touchPointsIdMapping = new TouchPointsIdMapping();
    _lastTouchPointId = 0;
}

- (osgViewer::GraphicsWindowIOS*) getGraphicsWindow {
    return _win;
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
- (id)initWithFrame:(CGRect)frame : (osgViewer::GraphicsWindowIOS*)win{
    
    _win = win;

    if ((self = [super initWithFrame:frame])) {
        // Get the layer
        CAEAGLLayer *eaglLayer = (CAEAGLLayer *)self.layer;
        
        osgViewer::GraphicsWindowIOS::WindowData* win_data(NULL);
        if (_win->getTraits()->inheritedWindowData.valid())
            win_data = dynamic_cast<osgViewer::GraphicsWindowIOS::WindowData*>(_win->getTraits()->inheritedWindowData.get());
        
        eaglLayer.opaque = win_data ? !win_data->getCreateTransparentView() : YES;
        bool retained_backing = win_data ? win_data->getUseRetainedBacking() : NO;

        if(_win->getTraits()->alpha > 0)
        {
            //create layer with alpha channel RGBA8
            eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
                                            [NSNumber numberWithBool:retained_backing], kEAGLDrawablePropertyRetainedBacking, kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat, nil];
        }else{
            //else no alpha, IOS uses RBG565
            eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
                                            [NSNumber numberWithBool:retained_backing], kEAGLDrawablePropertyRetainedBacking, kEAGLColorFormatRGB565, kEAGLDrawablePropertyColorFormat, nil];

        }
    }
    self.multipleTouchEnabled = YES;
    
    return self;
}

//
//Implement dealloc to destory our frame buffer
//
- (void) dealloc
{
    OSG_INFO << "GraphicsWindowIOSGLView::dealloc" << std::endl;
    if(_touchPointsIdMapping) 
        delete _touchPointsIdMapping;
    _touchPointsIdMapping = NULL;
    [super dealloc];
}

- (void)layoutSubviews {
    [super layoutSubviews];
    [self updateDimensions];
}


- (void) setFrame:(CGRect)frame
{
    [super setFrame:frame];
    [self updateDimensions];
}


- (void) updateDimensions
{
    if (_win)
    {
        CGRect frame = self.bounds;
        osg::Vec2 pointOrigin = osg::Vec2(frame.origin.x,frame.origin.y);
        osg::Vec2 pointSize = osg::Vec2(frame.size.width,frame.size.height);
        osg::Vec2 pixelOrigin = [(GraphicsWindowIOSGLView*)(self) convertPointToPixel:pointOrigin];
        osg::Vec2 pixelSize = [(GraphicsWindowIOSGLView*)(self) convertPointToPixel:pointSize];
        
        OSG_INFO << "updateDimensions, resize to "
            <<  pixelOrigin.x() << " " << pixelOrigin.y() << " " 
            << pixelSize.x() << " " << pixelSize.y() 
            << std::endl;
        _win->resized(pixelOrigin.x(), pixelOrigin.y(), pixelSize.x(), pixelSize.y());
    }
}

- (BOOL)createFramebuffer {

    _msaaFramebuffer = _msaaRenderBuffer = 0;
    
    glGenFramebuffersOES(1, &_viewFramebuffer);
    glGenRenderbuffersOES(1, &_viewRenderbuffer);
    
    // set the default id for osg to switch back after using fbos.
    _win->setDefaultFboId(_viewFramebuffer);
    
    glBindFramebufferOES(GL_FRAMEBUFFER_OES, _viewFramebuffer);
    glBindRenderbufferOES(GL_RENDERBUFFER_OES, _viewRenderbuffer);
    [_context renderbufferStorage:GL_RENDERBUFFER_OES fromDrawable:(CAEAGLLayer*)self.layer];
    glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_RENDERBUFFER_OES, _viewRenderbuffer);
    
    glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_WIDTH_OES, &_backingWidth);
    glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_HEIGHT_OES, &_backingHeight);
    
    osg::notify(osg::DEBUG_INFO) << "GraphicsWindowIOS::createFramebuffer INFO: Created GL RenderBuffer of size " << _backingWidth << ", " << _backingHeight << " ." << std::endl;

#if __IPHONE_OS_VERSION_MIN_REQUIRED >= 50000
    //on ios 5 we have to use a packed depth stencil buffer if we want stencil
    if(_win->getTraits()->depth > 0) {
        //add stencil if requested
        if(_win->getTraits()->stencil > 0) {
            // Create a packed depth stencil buffer.
            glGenRenderbuffersOES(1, &_depthRenderbuffer);
            glBindRenderbufferOES(GL_RENDERBUFFER_OES, _depthRenderbuffer);
            
            glRenderbufferStorageOES(GL_RENDERBUFFER_OES, GL_DEPTH24_STENCIL8_OES, _backingWidth, _backingHeight);
            
            glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_DEPTH_ATTACHMENT_OES,
                                         GL_RENDERBUFFER_OES, _depthRenderbuffer);
            glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_STENCIL_ATTACHMENT_OES,
                                         GL_RENDERBUFFER_OES, _depthRenderbuffer);
        }else{
            //normal depth buffer
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
    }
    
#else
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

#if defined(GL_DEPTH_COMPONENT32_OES)
        else if(_win->getTraits()->depth == 32){
            glRenderbufferStorageOES(GL_RENDERBUFFER_OES, GL_DEPTH_COMPONENT32_OES, _backingWidth, _backingHeight);
        }
#endif

        glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_DEPTH_ATTACHMENT_OES, GL_RENDERBUFFER_OES, _depthRenderbuffer);
    }
    
    //add stencil if requested
    if(_win->getTraits()->stencil > 0) {
        glGenRenderbuffersOES(1, &_stencilBuffer);
        glBindRenderbufferOES(GL_RENDERBUFFER_OES, _stencilBuffer);
        glRenderbufferStorageOES(GL_RENDERBUFFER_OES, GL_STENCIL_INDEX8_OES, _backingWidth, _backingHeight);
        glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_STENCIL_ATTACHMENT_OES, GL_RENDERBUFFER_OES, _stencilBuffer);
    }  
#endif
    
    //MSAA only available for >= 4.0 sdk
    
#if defined(__IPHONE_4_0) && (__IPHONE_OS_VERSION_MIN_REQUIRED >= __IPHONE_4_0)
    
    if(_win->getTraits()->sampleBuffers > 0) 
    {
        glGenFramebuffersOES(1, &_msaaFramebuffer); 
        glGenRenderbuffersOES(1, &_msaaRenderBuffer);
        
        _win->setDefaultFboId(_msaaFramebuffer);
        
        glBindFramebufferOES(GL_FRAMEBUFFER_OES, _msaaFramebuffer); 
        glBindRenderbufferOES(GL_RENDERBUFFER_OES, _msaaRenderBuffer);
        
        // Samples is the amount of pixels the MSAA buffer uses to make one pixel on the render // buffer. Use a small number like 2 for the 3G and below and 4 or more for newer models
        
        glRenderbufferStorageMultisampleAPPLE(GL_RENDERBUFFER_OES, _win->getTraits()->samples, GL_RGB5_A1_OES, _backingWidth, _backingHeight);
        
        glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_RENDERBUFFER_OES, _msaaRenderBuffer);
        glGenRenderbuffersOES(1, &_msaaDepthBuffer); 
        glBindRenderbufferOES(GL_RENDERBUFFER_OES, _msaaDepthBuffer);
        
        GLuint attachmentType = (_win->getTraits()->stencil > 0) ? GL_DEPTH24_STENCIL8_OES : ((_win->getTraits()->depth == 16) ? GL_DEPTH_COMPONENT16_OES : GL_DEPTH_COMPONENT24_OES);
        glRenderbufferStorageMultisampleAPPLE(GL_RENDERBUFFER_OES, _win->getTraits()->samples, attachmentType, _backingWidth , _backingHeight);
        
        glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_DEPTH_ATTACHMENT_OES, GL_RENDERBUFFER_OES, _msaaDepthBuffer);
        if (_win->getTraits()->stencil > 0) 
        {
            glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_STENCIL_ATTACHMENT_OES, GL_RENDERBUFFER_OES, _msaaDepthBuffer);
        }
    }
#endif
    
    if(glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES) != GL_FRAMEBUFFER_COMPLETE_OES) {
        OSG_FATAL << "GraphicsWindowIOS::createFramebuffer ERROR: Failed to create a GL RenderBuffer, glCheckFramebufferStatusOES returned '" 
                  << glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES) << "'." << std::endl;
        return NO;
    }
    
    return YES;
}


- (void)destroyFramebuffer {
    
    if(_viewFramebuffer)
    {
        glDeleteFramebuffersOES(1, &_viewFramebuffer);
        _viewFramebuffer = 0;
    }
    if(_viewRenderbuffer)
    {
        glDeleteRenderbuffersOES(1, &_viewRenderbuffer);
        _viewRenderbuffer = 0;
    }
    
    if(_depthRenderbuffer) {
        glDeleteRenderbuffersOES(1, &_depthRenderbuffer);
        _depthRenderbuffer = 0;
    }
    
    if(_stencilBuffer) {
        glDeleteRenderbuffersOES(1, &_stencilBuffer);
        _stencilBuffer = 0;
    }
    
    if(_msaaRenderBuffer) {
        glDeleteRenderbuffersOES(1, &_msaaRenderBuffer);
        _msaaRenderBuffer = 0;
    }
    
    if(_msaaDepthBuffer) {
        glDeleteRenderbuffersOES(1, &_msaaDepthBuffer);
        _msaaDepthBuffer = 0;
    }

    if(_msaaFramebuffer) {
        glDeleteFramebuffersOES(1, &_msaaFramebuffer);
        _msaaFramebuffer = 0;
    }
}

//
//Swap the view and render buffers
//
- (void)swapBuffers {


#if defined(__IPHONE_4_0) && (__IPHONE_OS_VERSION_MIN_REQUIRED >= __IPHONE_4_0)    
    if(_msaaFramebuffer) 
    {
        glBindFramebufferOES(GL_FRAMEBUFFER_OES, _msaaFramebuffer);
        
        glBindFramebufferOES(GL_READ_FRAMEBUFFER_APPLE, _msaaFramebuffer); 
        glBindFramebufferOES(GL_DRAW_FRAMEBUFFER_APPLE, _viewFramebuffer);
        
        glResolveMultisampleFramebufferAPPLE();
        
        GLenum attachments[] = {GL_DEPTH_ATTACHMENT_OES, GL_COLOR_ATTACHMENT0_OES}; 
        glDiscardFramebufferEXT(GL_READ_FRAMEBUFFER_APPLE, 2, attachments);
    }
#endif


      //swap buffers (sort of i think?)
    glBindRenderbufferOES(GL_RENDERBUFFER_OES, _viewRenderbuffer);
    
    //display render in context
    [_context presentRenderbuffer:GL_RENDERBUFFER_OES];
    
    //re bind the frame buffer for next frames renders
    glBindFramebufferOES(GL_FRAMEBUFFER_OES, _viewFramebuffer);
    
#if defined(__IPHONE_4_0) && (__IPHONE_OS_VERSION_MIN_REQUIRED >= __IPHONE_4_0)
    if (_msaaFramebuffer)
        glBindFramebufferOES(GL_FRAMEBUFFER_OES, _msaaFramebuffer);;
#endif
}

//
//bind view buffer as current for new render pass
//
- (void)bindFrameBuffer {

    //bind the frame buffer
    glBindFramebufferOES(GL_FRAMEBUFFER_OES, _viewFramebuffer);
    
#if defined(__IPHONE_4_0) && (__IPHONE_OS_VERSION_MIN_REQUIRED >= __IPHONE_4_0)
    if (_msaaFramebuffer)
        glBindFramebufferOES(GL_READ_FRAMEBUFFER_APPLE, _msaaFramebuffer);
#endif
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
    
    osg::ref_ptr<osgGA::GUIEventAdapter> osg_event(NULL);

    for(int i=0; i<[allTouches count]; i++)
    {
        
        UITouch *touch = [[allTouches allObjects] objectAtIndex:i];
        CGPoint pos = [touch locationInView:touch.view];
        osg::Vec2 pixelPos = [self convertPointToPixel: osg::Vec2(pos.x,pos.y)];
        unsigned int touch_id = [self computeTouchId: touch];
        
        if (!osg_event) {
            osg_event = _win->getEventQueue()->touchBegan(touch_id, [self convertTouchPhase: [touch phase]], pixelPos.x(), pixelPos.y());
        } else {
            osg_event->addTouchPoint(touch_id, [self convertTouchPhase: [touch phase]], pixelPos.x(), pixelPos.y());
        }
    }
    
    [super touchesBegan:touches withEvent:event];
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event {
    
    NSSet *allTouches = [event allTouches];
    
    osg::ref_ptr<osgGA::GUIEventAdapter> osg_event(NULL);

    for(int i=0; i<[allTouches count]; i++)
    {
        UITouch *touch = [[allTouches allObjects] objectAtIndex:i];
        CGPoint pos = [touch locationInView:touch.view];
        osg::Vec2 pixelPos = [self convertPointToPixel: osg::Vec2(pos.x,pos.y)];
        unsigned int touch_id = [self computeTouchId: touch];

        if (!osg_event) {
            osg_event = _win->getEventQueue()->touchMoved(touch_id, [self convertTouchPhase: [touch phase]], pixelPos.x(), pixelPos.y());
        } else {
            osg_event->addTouchPoint(touch_id, [self convertTouchPhase: [touch phase]], pixelPos.x(), pixelPos.y());
        }
    }
    
    [super touchesMoved:touches withEvent:event];    
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event

{    
    NSSet *allTouches = [event allTouches];
    
    osg::ref_ptr<osgGA::GUIEventAdapter> osg_event(NULL);
    
    for(int i=0; i<[allTouches count]; i++)
    {
        UITouch *touch = [[allTouches allObjects] objectAtIndex:i];
        CGPoint pos = [touch locationInView:touch.view];
        osg::Vec2 pixelPos = [self convertPointToPixel: osg::Vec2(pos.x,pos.y)];
        unsigned int touch_id = [self computeTouchId: touch];
        if (!osg_event) {
            osg_event = _win->getEventQueue()->touchEnded(touch_id, [self convertTouchPhase: [touch phase]], pixelPos.x(), pixelPos.y(), [touch tapCount]);
        } else {
            osg_event->addTouchPoint(touch_id, [self convertTouchPhase: [touch phase]], pixelPos.x(), pixelPos.y(), [touch tapCount]);
        }
    }
    
    [super touchesEnded:touches withEvent:event];    
}

-(void) touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event 
{
    [self touchesEnded: touches withEvent:event];
}

@end



@interface GraphicsWindowIOSGLViewController : UIViewController
{

}
- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation;
- (void)willAnimateRotationToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation duration:(NSTimeInterval)duration;

@end

@implementation GraphicsWindowIOSGLViewController


- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    osgViewer::GraphicsWindowIOS* win = [(GraphicsWindowIOSGLView*)(self.view) getGraphicsWindow];
    if(!win){return  NO;}
    
    osgViewer::GraphicsWindowIOS::WindowData::DeviceOrientationFlags flags = win->getDeviceOrientationFlags();
    
 
    BOOL result(NO);
    
    switch (interfaceOrientation) {
        case UIDeviceOrientationPortrait:
            if(flags & osgViewer::GraphicsWindowIOS::WindowData::PORTRAIT_ORIENTATION){
                result = YES;
            }
            break;
        case UIDeviceOrientationPortraitUpsideDown:
            if(flags & osgViewer::GraphicsWindowIOS::WindowData::PORTRAIT_UPSIDEDOWN_ORIENTATION){
                result = YES;
            }
            break;
        case UIInterfaceOrientationLandscapeLeft:
            if(win->getTraits()->supportsResize && flags & osgViewer::GraphicsWindowIOS::WindowData::LANDSCAPE_LEFT_ORIENTATION){
                result = YES;
            }
            break;
        case UIInterfaceOrientationLandscapeRight:
            if(win->getTraits()->supportsResize && flags & osgViewer::GraphicsWindowIOS::WindowData::LANDSCAPE_RIGHT_ORIENTATION){
                result = YES;
            }
            break;
        default:
            break;
    }
    OSG_INFO << "shouldAutorotateToInterfaceOrientation for " << interfaceOrientation << ": " << ((result==YES) ? "YES" : "NO") << std::endl;
    return result;
}


- (void)willAnimateRotationToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation duration:(NSTimeInterval)duration 
{
    [(GraphicsWindowIOSGLView*)(self.view) updateDimensions];
}



@end



using namespace osgIOS; 
namespace osgViewer {

    
    
#pragma mark GraphicsWindowIOS



// ----------------------------------------------------------------------------------------------------------
// init
// ----------------------------------------------------------------------------------------------------------

void GraphicsWindowIOS::init()
{
    if (_initialized) return;

    _ownsWindow = false;
    _context = NULL;
    _window = NULL;
    _view = NULL;
    _viewController = NULL;
    
    _updateContext = true;
    //if -1.0 we use the screens scale factor
    _viewContentScaleFactor = -1.0f;
    _valid = _initialized = true;

    // make sure the event queue has the correct window rectangle size and input range
    getEventQueue()->syncWindowRectangleWithGraphcisContext();
}


// ----------------------------------------------------------------------------------------------------------
// realizeImplementation, creates the window + context
// ----------------------------------------------------------------------------------------------------------

bool GraphicsWindowIOS::realizeImplementation()
{
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
    
    BOOL bar_hidden = (_traits->windowDecoration) ? NO: YES;
    #ifdef __IPHONE_OS_VERSION_MIN_REQUIRED
    #if __IPHONE_OS_VERSION_MIN_REQUIRED > 30100
        [[UIApplication sharedApplication] setStatusBarHidden: bar_hidden withAnimation:UIStatusBarAnimationNone];
    #else
        [[UIApplication sharedApplication] setStatusBarHidden: bar_hidden animated:NO];
    #endif
    #endif
    
    //Get info about the requested screen
    IOSWindowingSystemInterface* wsi = dynamic_cast<IOSWindowingSystemInterface*>(osg::GraphicsContext::getWindowingSystemInterface());
    osg::Vec2 screenSizePoints;
    osg::Vec2 screenSizePixels;
    float screenScaleFactor = 1.0f;
    UIScreen* screen = nil;
    osg::GraphicsContext::ScreenSettings screenSettings;
    if (wsi) {
        wsi->getScreenContentScaleFactor((*_traits), screenScaleFactor);
        wsi->getScreenSizeInPoints((*_traits), screenSizePoints); 
        screenSizePixels = osg::Vec2(screenSettings.width, screenSettings.height);
        wsi->getScreenSettings((*_traits), screenSettings);
        screen = wsi->getUIScreen((*_traits));
    }else{
        OSG_FATAL << "GraphicsWindowIOS::realizeImplementation: ERROR: Failed to create IOS windowing system, OSG will be unable to create a vaild gl context and will not be able to render." << std::endl;
        return false;
    }
    
    _ownsWindow = true;
    
    // see if an existing inherited window was passed in
    WindowData* windowData = _traits->inheritedWindowData ? dynamic_cast<WindowData*>(_traits->inheritedWindowData.get()) : NULL;
    if (windowData) 
    {
        if (windowData->getWindowOrParentView())
        {
            _ownsWindow = false;        
            _window = windowData->getWindowOrParentView();
        }
        
        _deviceOrientationFlags = windowData->_deviceOrientationFlags;
        _viewContentScaleFactor = windowData->_viewContentScaleFactor;
    } 
    
    //if the user hasn't specified a viewScaleFactor we will use the screens scale factor
    //so we get a full res buffer
    if(_viewContentScaleFactor < 0.0f)
    {_viewContentScaleFactor = screenScaleFactor;}
    

    OSG_DEBUG << "GraphicsWindowIOS::realizeImplementation / ownsWindow: " << _ownsWindow << std::endl;

    
    //Here's the confusing bit, the default traits use the screen res which is in pixels and the user will want to use pixels also
    //but we need to create our views and windows in points. By default we create a full res buffer across all devices. This
    //means that for backward compatibility you need to set the windowData _viewContentScaleFactor to 1.0f and set the screen res to the
    //res of the older gen device.
    CGRect window_bounds;
    osg::Vec2 pointsOrigin = this->pixelToPoint(osg::Vec2(_traits->x, _traits->y));
    osg::Vec2 pointsSize = this->pixelToPoint(osg::Vec2(_traits->width, _traits->height));

    window_bounds.origin.x = pointsOrigin.x(); 
    window_bounds.origin.y = pointsOrigin.y();
    window_bounds.size.width = pointsSize.x(); 
    window_bounds.size.height = pointsSize.y();
    
    
    //if we own the window we need to create one
    if (_ownsWindow) 
    {
        //create the IOS window object using the viewbounds (in points) required for our context size
        _window = [[GraphicsWindowIOSWindow alloc] initWithFrame: window_bounds];// styleMask: style backing: NSBackingStoreBuffered defer: NO];
        
        if (!_window) {
            OSG_WARN << "GraphicsWindowIOS::realizeImplementation: ERROR: Failed to create GraphicsWindowIOSWindow can not display gl view" << std::endl;
            return false;
        }
        
        OSG_DEBUG << "GraphicsWindowIOS::realizeImplementation: INFO: Created UIWindow with bounds '" << window_bounds.size.width << ", " << window_bounds.size.height << "' (points)." << std::endl;
        
        //if the user has requested a differnet screenNum from default 0 get the UIScreen object and
        //apply to our window (this is for IPad external screens, I don't have one, so I've no idea if it works)
        //I'm also not sure if we should apply this to external windows also?
        if(_traits->screenNum > 0 && screen != nil)
        {
            _window.screen = screen;
        }
    } 
            
    //create the desired OpenGLES context type
#if OSG_GLES1_FEATURES
    _context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1];
#elif OSG_GLES2_FEATURES
    _context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
#endif
    
    if (!_context || ![EAGLContext setCurrentContext:_context]) {
        
        #if OSG_GLES1_FEATURES
        OSG_FATAL << "GraphicsWindowIOS::realizeImplementation: ERROR: Failed to create a valid OpenGLES1 context" << std::endl;
        #elif OSG_GLES2_FEATURES
        OSG_FATAL << "GraphicsWindowIOS::realizeImplementation: ERROR: Failed to create a valid OpenGLES2 context" << std::endl;
        #endif
        return false;
    }

    //create the view to display our context in our window
    CGRect gl_view_bounds = (_ownsWindow) ? [_window frame] : window_bounds;
    GraphicsWindowIOSGLView* theView = [[ GraphicsWindowIOSGLView alloc ] initWithFrame: gl_view_bounds : this ];
    if(!theView)
    {
        OSG_FATAL << "GraphicsWindowIOS::realizeImplementation: ERROR: Failed to create GraphicsWindowIOSGLView, can not create frame buffers." << std::endl;
        return false;
    }
    
    [theView setAutoresizingMask:  ( UIViewAutoresizingFlexibleWidth |  UIViewAutoresizingFlexibleHeight) ];
    
    //Apply our content scale factor to our view, this is what converts the views points
    //size to our desired context size.
#if defined(__IPHONE_4_0) && (__IPHONE_OS_VERSION_MIN_REQUIRED >= __IPHONE_4_0)
    theView.contentScaleFactor = _viewContentScaleFactor;
#endif    
    [theView setGraphicsWindow: this];
    [theView setOpenGLContext:_context];
    _view = theView;
    
    OSG_DEBUG << "GraphicsWindowIOS::realizeImplementation / view: " << theView << std::endl;

    if (getDeviceOrientationFlags() != WindowData::IGNORE_ORIENTATION) 
    {
        _viewController = [[GraphicsWindowIOSGLViewController alloc] init];
        _viewController.view = _view;
    }
    
    // Attach view to window
    [_window addSubview: _view];
    if ([_window isKindOfClass:[UIWindow class]])
        _window.rootViewController = _viewController;
    [theView release];
    
    //if we own the window also make it visible
    if (_ownsWindow) 
    {
        
        //show window
        [_window makeKeyAndVisible];
    }

    [pool release];
    
    // IOSs origin is top/left:
    getEventQueue()->getCurrentEventState()->setMouseYOrientation(osgGA::GUIEventAdapter::Y_INCREASING_DOWNWARDS);

    // make sure the event queue has the correct window rectangle size and input range
    getEventQueue()->syncWindowRectangleWithGraphcisContext();
    
    _valid = _initialized = _realized = true;
    return _valid;
}




// ----------------------------------------------------------------------------------------------------------
// closeImplementation
// ----------------------------------------------------------------------------------------------------------
void GraphicsWindowIOS::closeImplementation()
{
    OSG_INFO << "close IOS window" << std::endl;
    _valid = false;
    _realized = false;
   
    
    if (_view) 
    {
        [_view setOpenGLContext: NULL];
        [_context release];
        [_view removeFromSuperview];
        [_view setGraphicsWindow: NULL];
    }
    
    if (_viewController) 
    {
        [_viewController release];
        _viewController = NULL;
    }
        
    if (_window && _ownsWindow) 
    {  
        [_window release];
        //[glView release];
    }

    
    _window = NULL;
    _view = NULL;  
    _context = NULL;  
}


// ----------------------------------------------------------------------------------------------------------
// makeCurrentImplementation
// ----------------------------------------------------------------------------------------------------------

bool GraphicsWindowIOS:: makeCurrentImplementation()
{
    
    
    //bind the context
    [EAGLContext setCurrentContext:_context];
    
    if (_updateContext)
    {
        [_view destroyFramebuffer];
        [_view createFramebuffer];

        _updateContext = false; 
    }
    //i think we also want to bind the frame buffer here
    //[_view bindFrameBuffer];

    return true;
}


// ----------------------------------------------------------------------------------------------------------
// releaseContextImplementation
// ----------------------------------------------------------------------------------------------------------

bool GraphicsWindowIOS::releaseContextImplementation()
{
    if ([EAGLContext currentContext] == _context) {
        [EAGLContext setCurrentContext:nil];
    }
    return true;
}


// ----------------------------------------------------------------------------------------------------------
// swapBuffersImplementation
// ----------------------------------------------------------------------------------------------------------

void GraphicsWindowIOS::swapBuffersImplementation()
{
    //[_context flushBuffer];
    [_view swapBuffers];
}



// ----------------------------------------------------------------------------------------------------------
// setWindowDecorationImplementation
//
// We will use this to toggle the status bar on IPhone, nearest thing to window decoration
// ----------------------------------------------------------------------------------------------------------

bool GraphicsWindowIOS::setWindowDecorationImplementation(bool flag)
{
    if (!_realized || !_ownsWindow) return false;

    BOOL bar_hidden = (flag) ? NO: YES;
    #ifdef __IPHONE_OS_VERSION_MIN_REQUIRED
    #if __IPHONE_OS_VERSION_MIN_REQUIRED > 30100
        [[UIApplication sharedApplication] setStatusBarHidden: bar_hidden withAnimation:UIStatusBarAnimationNone];
    #else
        [[UIApplication sharedApplication] setStatusBarHidden: bar_hidden animated:NO];
    #endif
    #endif
    
    return true;
}


// ----------------------------------------------------------------------------------------------------------
// grabFocus
// ----------------------------------------------------------------------------------------------------------
void GraphicsWindowIOS::grabFocus()
{
    //i think make key is the equivalent of focus on iphone 
    [_window makeKeyWindow];
}


// ----------------------------------------------------------------------------------------------------------
// grabFocusIfPointerInWindow
// ----------------------------------------------------------------------------------------------------------
void GraphicsWindowIOS::grabFocusIfPointerInWindow()
{
    OSG_INFO << "GraphicsWindowIOS :: grabFocusIfPointerInWindow not implemented yet " << std::endl;
}

// ----------------------------------------------------------------------------------------------------------
// raiseWindow
// Raise the window to the top.
// ----------------------------------------------------------------------------------------------------------
void GraphicsWindowIOS::raiseWindow()
{
    [_window bringSubviewToFront:_view];
}

// ----------------------------------------------------------------------------------------------------------
// resizedImplementation
// ----------------------------------------------------------------------------------------------------------

void GraphicsWindowIOS::resizedImplementation(int x, int y, int width, int height)
{
    GraphicsContext::resizedImplementation(x, y, width, height);
    
    _updateContext = true;
    
    getEventQueue()->windowResize(x,y,width, height, getEventQueue()->getTime());
}




// ----------------------------------------------------------------------------------------------------------
// setWindowRectangleImplementation
// ----------------------------------------------------------------------------------------------------------
bool GraphicsWindowIOS::setWindowRectangleImplementation(int x, int y, int width, int height)
{
    OSG_INFO << "GraphicsWindowIOS :: setWindowRectangleImplementation not implemented yet " << std::endl;
    if (!_ownsWindow)
        return false;
            
    return true;
}

    
bool GraphicsWindowIOS::checkEvents()
{

    return !(getEventQueue()->empty());
}



// ----------------------------------------------------------------------------------------------------------
// setWindowName
// ----------------------------------------------------------------------------------------------------------

void GraphicsWindowIOS::setWindowName (const std::string & name)
{
    OSG_INFO << "GraphicsWindowIOS :: setWindowName not implemented yet " << std::endl;
}


// ----------------------------------------------------------------------------------------------------------
// useCursor, no cursor on IOS
// ----------------------------------------------------------------------------------------------------------

void GraphicsWindowIOS::useCursor(bool cursorOn)
{
    OSG_INFO << "GraphicsWindowIOS :: useCursor not implemented yet " << std::endl;
}


// ----------------------------------------------------------------------------------------------------------
// setCursor, no cursor on IOS
// ----------------------------------------------------------------------------------------------------------

void GraphicsWindowIOS::setCursor(MouseCursor mouseCursor)
{
    OSG_INFO << "GraphicsWindowIOS :: setCursor not implemented yet " << std::endl;
}


// ----------------------------------------------------------------------------------------------------------
// setVSync, no vsync on IOS
// ----------------------------------------------------------------------------------------------------------

void GraphicsWindowIOS::setVSync(bool f) 
{
    OSG_INFO << "GraphicsWindowIOS :: setVSync not implemented yet " << std::endl;
}
    
    
// ----------------------------------------------------------------------------------------------------------
// helper funcs for converting points to pixels taking into account the views contents scale factor
// ----------------------------------------------------------------------------------------------------------

osg::Vec2 GraphicsWindowIOS::pointToPixel(const osg::Vec2& point)
{
    return point * _viewContentScaleFactor;
}
    
osg::Vec2 GraphicsWindowIOS::pixelToPoint(const osg::Vec2& pixel)
{
    float scaler = 1.0f / _viewContentScaleFactor;
    return pixel * scaler;
}


// ----------------------------------------------------------------------------------------------------------
// d'tor
// ----------------------------------------------------------------------------------------------------------

GraphicsWindowIOS::~GraphicsWindowIOS() 
{
    close();
}



class ConcreteIOSWindowingSystemInterface : public  IOSWindowingSystemInterface {
public:
    ConcreteIOSWindowingSystemInterface()
    :    IOSWindowingSystemInterface()
    {
    }
    
    virtual osg::GraphicsContext* createGraphicsContext(osg::GraphicsContext::Traits* traits) 
    {
        if (traits->pbuffer)
        {
            // pbuffers not supported on iOS
            return 0;
        }
        else
        {
            osg::ref_ptr<GraphicsWindowIOS> window = new GraphicsWindowIOS(traits);
            if (window->valid()) return window.release();
            else return 0;
        }
    }
};

}//end namspace


RegisterWindowingSystemInterfaceProxy<osgViewer::ConcreteIOSWindowingSystemInterface> createWindowingSystemInterfaceProxy;


// declare C entry point for static compilation.
extern "C" void graphicswindow_IOS(void)
{
    osg::GraphicsContext::setWindowingSystemInterface(new osgViewer::ConcreteIOSWindowingSystemInterface());
}
