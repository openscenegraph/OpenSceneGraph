/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2008 Robert Osfield
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

#include <osgDB/FileNameUtils>

#include <QTKit/QTKit.h>
#include <QTKit/QTTime.h>
#include <Quicktime/Quicktime.h>

#include "OSXQTKitVideo.h"
#include "OSXCoreVideoAdapter.h"
#include "OSXCoreVideoTexture.h"

namespace {

static NSString* toNSString(const std::string& str)
{
    return [NSString stringWithUTF8String: str.c_str()];
}

static std::string toString(NSString* str)
{
    return str ? std::string([str UTF8String]) : "";
}


class NSAutoreleasePoolHelper {
public:
    NSAutoreleasePoolHelper()
    {
        _pool = [[NSAutoreleasePool alloc] init];
    }

    ~NSAutoreleasePoolHelper()
    {
        [_pool drain];
    }

private:
    NSAutoreleasePool* _pool;
};

}

@interface NotificationHandler : NSObject {
    OSXQTKitVideo* video;
}

@property (readwrite,assign) OSXQTKitVideo* video;

- (void) movieNaturalSizeDidChange:(NSNotification*)the_notification;
- (void) movieLoadStateDidChange:(NSNotification*)the_notification;

@end

@implementation NotificationHandler

@synthesize video;

- (void) movieNaturalSizeDidChange:(NSNotification*)the_notification
{
    video->requestNewFrame(true);
}

- (void) movieLoadStateDidChange:(NSNotification*)the_notification
{
    video->requestNewFrame(true);
}


@end


struct OSXQTKitVideo::Data {
    QTVisualContextRef visualContext;
    CVPixelBufferRef lastFrame;
    NotificationHandler* notificationHandler;
    Data() : visualContext(NULL), lastFrame(NULL) {}
};


void OSXQTKitVideo::initializeQTKit()
{
    static bool inited(false);
    if (!inited)
    {
        inited = true;
        // force initialization of QTKit on the main-thread!
        if (![NSThread isMainThread]) {
            dispatch_apply(1, dispatch_get_main_queue(), ^(size_t n) {
                EnterMovies();
                {
                    // workaround for gcc bug. See discussion here
                    // http://stackoverflow.com/questions/6525928/objective-c-block-vs-objective-c-block
                    
                    #if (GCC_VERSION <= 40201) && !(__clang__)
                        QTMovie* ::temp_movie = [QTMovie movie];
                    #else
                        QTMovie* temp_movie = [QTMovie movie];
                    #endif
                    
                    // release missing by intent, gets released by the block!
                    temp_movie = NULL;
                }
            });
        }
        else
        {
            EnterMovies();
            QTMovie* movie = [QTMovie movie];
            movie = NULL;
        }
    }
}


OSXQTKitVideo::OSXQTKitVideo()
    : osgVideo::VideoImageStream()
    , _rate(0.0)
    , _coreVideoAdapter(NULL)
{
    NSAutoreleasePoolHelper autorelease_pool_helper;
     
    initializeQTKit();
    
    _status = INVALID;
    _data = new Data();
    _data->notificationHandler = [[NotificationHandler alloc] init];
    _data->notificationHandler.video = this;
    
    setOrigin(osg::Image::TOP_LEFT);
}


OSXQTKitVideo::~OSXQTKitVideo()
{
    _status = INVALID;
    
    NSAutoreleasePoolHelper autorelease_pool_helper;
    
    [[NSNotificationCenter defaultCenter] removeObserver:_data->notificationHandler
                name:QTMovieLoadStateDidChangeNotification object:_movie];
            
    [[NSNotificationCenter defaultCenter] removeObserver:_data->notificationHandler
#if defined(MAC_OS_X_VERSION_10_6) && (MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_6)
        name:QTMovieNaturalSizeDidChangeNotification
#else
        name:QTMovieSizeDidChangeNotification
#endif    
        object:_movie];

    [_movie stop];
    [_movie invalidate];
    [_movie release];
    
    if (_data->visualContext)
        QTVisualContextRelease(_data->visualContext);
    
    if (_data->lastFrame)
    {
        CFRelease(_data->lastFrame);
        CVPixelBufferRelease(_data->lastFrame);
    }
    
    [_data->notificationHandler release];
    
    delete _data;
}
    
  

void OSXQTKitVideo::open(const std::string& file_name)
{
    bool valid = true;
    NSAutoreleasePoolHelper autorelease_pool_helper;
    
    NSError* error;
    
    NSMutableDictionary* movieAttributes = [NSMutableDictionary dictionaryWithObjectsAndKeys:
        [NSNumber numberWithBool:NO], QTMovieOpenAsyncOKAttribute,
        nil];

    if (osgDB::containsServerAddress(file_name))
        [movieAttributes setObject:[NSURL URLWithString: toNSString(file_name)] forKey: QTMovieURLAttribute];
    else
        [movieAttributes setObject:[NSURL fileURLWithPath: toNSString(file_name)] forKey: QTMovieURLAttribute];
        
        
    _movie = [[QTMovie alloc] initWithAttributes:movieAttributes 
                                           error: &error];

    if(error || _movie == NULL)
    {
        OSG_WARN << "OSXQTKitVideo: could not load movie from " << file_name << std::endl;
        valid = false;
    }

    NSSize movie_size = [[_movie attributeForKey:QTMovieNaturalSizeAttribute] sizeValue];

    QTGetTimeInterval([_movie duration], &_duration);
    

    NSDictionary *pixelBufferAttributes = [NSDictionary dictionaryWithObjectsAndKeys:
        //in general this shouldn't be forced. but in order to ensure we get good pixels use this one
        [NSNumber numberWithInt: kCVPixelFormatType_32BGRA], (NSString*)kCVPixelBufferPixelFormatTypeKey,
        [NSNumber numberWithInteger:1], kCVPixelBufferBytesPerRowAlignmentKey,
        [NSNumber numberWithBool:YES], kCVPixelBufferOpenGLCompatibilityKey,
        //specifying width and height can't hurt since we know
        nil];

    NSMutableDictionary *ctxAttributes = [NSMutableDictionary dictionaryWithObject:pixelBufferAttributes 
        forKey:(NSString*)kQTVisualContextPixelBufferAttributesKey];

    OSStatus err = QTPixelBufferContextCreate(kCFAllocatorDefault, (CFDictionaryRef)ctxAttributes, &_data->visualContext);
    if(err)
    {
        OSG_WARN << "OSXQTKitVideo: could not create Pixel Buffer: " << err << std::endl;
        valid = false;
    }
    
    allocateImage((int)movie_size.width,(int)movie_size.height,1,GL_BGRA,GL_UNSIGNED_INT_8_8_8_8_REV,1);
     
    setInternalTextureFormat(GL_RGBA8);
    
    SetMovieVisualContext([_movie quickTimeMovie], _data->visualContext);
    
    [[NSNotificationCenter defaultCenter] addObserver:_data->notificationHandler
        selector:@selector(movieNaturalSizeDidChange:)
#if defined(MAC_OS_X_VERSION_10_6) && (MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_6)
        name:QTMovieNaturalSizeDidChangeNotification
#else
        name:QTMovieSizeDidChangeNotification
#endif    
        object:_movie];

    [[NSNotificationCenter defaultCenter] addObserver:_data->notificationHandler
        selector:@selector(movieLoadStateDidChange:)
        name:QTMovieLoadStateDidChangeNotification
        object:_movie];
    
    applyLoopingMode();
    
    _waitForFirstFrame = true;
    requestNewFrame(true);
    _fileName = file_name;
    _status = (valid) ? PAUSED : INVALID;
}



void OSXQTKitVideo::setTimeMultiplier(double r)
{
    if (!valid())
        return;
    
    NSAutoreleasePoolHelper pool;
    _rate = r;
    [_movie setRate: _rate];
    
    _status = (_rate != 0) ? PLAYING : PAUSED;
    setNeedsDispatching( _status == PLAYING ? RequestContinuousUpdate : StopUpdate );
}


double OSXQTKitVideo::getTimeMultiplier() const
{
    NSAutoreleasePoolHelper pool;
    _rate = [_movie rate];
    return _rate;
}


void OSXQTKitVideo::setVolume (float f)
{
    NSAutoreleasePoolHelper pool;
    
    [_movie setVolume: f];
}



float OSXQTKitVideo::getVolume () const
{
    NSAutoreleasePoolHelper pool;
    
    return [_movie volume];
}


    
float OSXQTKitVideo::getAudioBalance()
{
    float balance;
    GetMovieAudioBalance([_movie quickTimeMovie], &balance, 0);
    return balance;
}


void OSXQTKitVideo::setAudioBalance(float b)
{
    SetMovieAudioBalance([_movie quickTimeMovie], b, 0);
}


void OSXQTKitVideo::seek (double t)
{
    NSAutoreleasePoolHelper pool;
    [_movie setCurrentTime: QTMakeTimeWithTimeInterval(t)];
    if (!isPlaying())
        requestNewFrame(true);
}


void OSXQTKitVideo::play ()
{
    setTimeMultiplier(1.0);
}


void OSXQTKitVideo::pause ()
{
    setTimeMultiplier(0.0);
}

void OSXQTKitVideo::applyLoopingMode()
{
    NSAutoreleasePoolHelper pool;
    [_movie setAttribute:[NSNumber numberWithBool:(getLoopingMode() == LOOPING) ] forKey:QTMovieLoopsAttribute];
}


double OSXQTKitVideo::getCurrentTime() const
{
    double t;
    QTGetTimeInterval([_movie currentTime], &t);
    return t;
}



void OSXQTKitVideo::setCoreVideoAdapter(OSXCoreVideoAdapter* adapter)
{    
    _coreVideoAdapter = adapter;
    SetMovieVisualContext([_movie quickTimeMovie], _coreVideoAdapter ? _coreVideoAdapter->getVisualContext() : _data->visualContext  );
}




void OSXQTKitVideo::decodeFrame(bool force)
{
    if(getCoreVideoAdapter())
        return;
    
    
    QTVisualContextTask(_data->visualContext);
    
    CVPixelBufferRef currentFrame(NULL);
    const CVTimeStamp* in_output_time(NULL);
    
    if(!force && !QTVisualContextIsNewImageAvailable(_data->visualContext, in_output_time))
      return;
        
    OSStatus error_status = QTVisualContextCopyImageForTime(_data->visualContext, kCFAllocatorDefault, in_output_time, &currentFrame);
    
    if ((noErr == error_status) && (NULL != currentFrame))
    {
        if (_waitForFirstFrame) {
            _waitForFirstFrame = false;
        }
        
        if (_data->lastFrame) {
            CFRelease(_data->lastFrame);
            CVPixelBufferRelease(_data->lastFrame);
        }
        size_t bpr = CVPixelBufferGetBytesPerRow(currentFrame);
        size_t buffer_width =  CVPixelBufferGetWidth(currentFrame);
        size_t buffer_height = CVPixelBufferGetHeight(currentFrame);
        
        CVPixelBufferLockBaseAddress( currentFrame, kCVPixelBufferLock_ReadOnly );

        void* raw_pixel_data = CVPixelBufferGetBaseAddress(currentFrame);

        
        setImage(buffer_width,buffer_height,1,
                             GL_RGBA8,
                             GL_BGRA,
                             GL_UNSIGNED_BYTE,
                             (unsigned char *)raw_pixel_data,
                             osg::Image::NO_DELETE,1, bpr/4);
                             
        CVPixelBufferUnlockBaseAddress( currentFrame, 0 );
        
        _data->lastFrame = currentFrame;
        CFRetain(_data->lastFrame);
        dirty();
    }
}


osg::Texture* OSXQTKitVideo::createSuitableTexture()
{
    return new OSXCoreVideoTexture(this);
}
