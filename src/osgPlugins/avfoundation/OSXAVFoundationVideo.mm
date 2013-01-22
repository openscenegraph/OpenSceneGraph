#include "OSXAVFoundationVideo.h"

#include <osgDB/FileNameUtils>
#include <osg/ValueObject>
#include <iostream>
#include <deque>

#import <AVFoundation/AVFoundation.h>
#import "TargetConditionals.h" 
#if (TARGET_OS_IPHONE)
#import <UIKit/UIKit.h>
#include <osgViewer/api/IOS/GraphicsWindowIOS>
#else
#import <Cocoa/Cocoa.h>
#include <osgViewer/api/Cocoa/GraphicsWindowCocoa>
#endif
#include "OSXAVFoundationCoreVideoTexture.h"



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
        [_pool release];
    }

private:
    NSAutoreleasePool* _pool;
};
}

@interface AVPlayer (MOAdditions)
- (NSURL *)currentURL;
- (void)setVolume:(CGFloat)volume;
@end;

@implementation AVPlayer (MOAdditions)

- (NSURL *)currentURL {
    AVAsset *asset = self.currentItem.asset;
    if ([asset isMemberOfClass:[AVURLAsset class]])
        return ((AVURLAsset *)asset).URL;
    return nil;
}

- (void)setVolume:(CGFloat)volume {
    NSArray *audioTracks = [self.currentItem.asset tracksWithMediaType:AVMediaTypeAudio];
    NSMutableArray *allAudioParams = [NSMutableArray array];
    for (AVAssetTrack *track in audioTracks) {
        AVMutableAudioMixInputParameters *audioInputParams = [AVMutableAudioMixInputParameters audioMixInputParameters];
        [audioInputParams setVolume:volume atTime:kCMTimeZero];
        [audioInputParams setTrackID:[track trackID]];
        [allAudioParams addObject:audioInputParams];
    }
    AVMutableAudioMix *audioMix = [AVMutableAudioMix audioMix];
    [audioMix setInputParameters:allAudioParams];
    [self.currentItem setAudioMix:audioMix];
}

@end

@interface OSXAVFoundationVideoDelegate : NSObject {
    OSXAVFoundationVideo* video;
}
@property (readwrite,assign) OSXAVFoundationVideo* video;

- (void) playerItemDidReachEnd:(NSNotification*)the_notification;

@end;

@implementation OSXAVFoundationVideoDelegate

@synthesize video;

- (void) playerItemDidReachEnd:(NSNotification*)the_notification
{
    if (video->getLoopingMode() == osg::ImageStream::LOOPING) {
        video->seek(0);
    }
    else {
        video->pause();
    }
}

@end



class OSXAVFoundationVideo::Data {
public:
    AVPlayer* avplayer;
    AVPlayerItemVideoOutput* output;
    OSXAVFoundationVideoDelegate* delegate;
    std::vector<CVBufferRef> lastFrames;
    int readFrameNdx, writeFrameNdx;
    #if (TARGET_OS_IPHONE)
    CVOpenGLESTextureCacheRef coreVideoTextureCache;
    #else
    CVOpenGLTextureCacheRef coreVideoTextureCache;
    #endif
    
    Data()
        : avplayer(NULL)
        , output(NULL)
        , delegate(NULL)
        , lastFrames(3)
        , readFrameNdx(0)
        , writeFrameNdx(0)
        , coreVideoTextureCache(0)
    {
    }
    
    void clear()
    {
        if (delegate) {
            [[NSNotificationCenter defaultCenter] removeObserver: delegate
                name:AVPlayerItemDidPlayToEndTimeNotification
                object:avplayer.currentItem
            ];
            [delegate release];
        }
        
        if (avplayer) {
            [avplayer cancelPendingPrerolls];
            [avplayer.currentItem.asset cancelLoading];
            [avplayer.currentItem removeOutput:output];
        }
        
        [output release];
        [avplayer release];
        
        
        avplayer = NULL;
        output = NULL;
        delegate = NULL;
    }
    
    ~Data() {
        
        clear();
        
        for(unsigned int i=0; i< lastFrames.size(); ++i)
        {
            if (lastFrames[i])
            {
                CVBufferRelease(lastFrames[i]);
            }
        }
        
        if (coreVideoTextureCache)
        {
            #if (TARGET_OS_IPHONE)
            CFRelease(coreVideoTextureCache); // huh, there's no CVOpenGLESTextureCacheRelease?
            #else
            CVOpenGLTextureCacheRelease(coreVideoTextureCache);
            #endif
            coreVideoTextureCache = NULL;
        }
    }
    
    void addFrame(CVBufferRef frame)
    {
        unsigned int new_ndx = writeFrameNdx + 1;
        
        if (new_ndx >= lastFrames.size())
            new_ndx = 0;
        
        if (new_ndx == readFrameNdx) {
            new_ndx = readFrameNdx+1;
            if (new_ndx >= lastFrames.size())
                new_ndx = 0;
        }
        
        if (lastFrames[new_ndx])
        {
            CVBufferRelease(lastFrames[new_ndx]);
        }
        
        lastFrames[new_ndx] = frame;
        
        writeFrameNdx = new_ndx;
        //std::cout << "new frame: " << writeFrameNdx << std::endl;
    }
    
    bool hasNewFrame() const {
        return readFrameNdx != writeFrameNdx;
    }
    
    CVBufferRef getLastFrame() {
        readFrameNdx = writeFrameNdx;
        // std::cout << "get frame: " << readFrameNdx << std::endl;
        return lastFrames[readFrameNdx];
    }
};

OSXAVFoundationVideo::OSXAVFoundationVideo()
    : osgVideo::VideoImageStream()
    , _volume(1.0)
    , _fileOpened(false)
    , _useCoreVideo(false)
    , _dimensionsChangedCallbackNeeded(false)
{
    _data = new Data();
    _status = INVALID;
    setOrigin(TOP_LEFT);
    
    // std::cout << " OSXAVFoundationVideo " << this << std::endl;
}


OSXAVFoundationVideo::~OSXAVFoundationVideo()
{
    // std::cout << "~OSXAVFoundationVideo " << this << " " << _data->avplayer << std::endl;
    quit();
    clear();
    if (_data)
        delete _data;
}
        
void OSXAVFoundationVideo::play()
{
    if (_data->avplayer) {
        [_data->avplayer play];
        _status = PLAYING;
        setNeedsDispatching(RequestContinuousUpdate);
    }
}


void OSXAVFoundationVideo::setTimeMultiplier(double rate)
{
    if (_data->avplayer)
    {
        _data->avplayer.rate = rate;
        _status = (rate != 0.0) ? PLAYING : PAUSED;
        setNeedsDispatching(_status == PLAYING ? RequestContinuousUpdate: StopUpdate);
    }
}

double OSXAVFoundationVideo::getTimeMultiplier() const
{
    return _data->avplayer ? _data->avplayer.rate : 0.0f;
}
    

void OSXAVFoundationVideo::pause()
{
    setNeedsDispatching(StopUpdate);
    
    NSAutoreleasePoolHelper helper;

    if (_data->avplayer) {
        [_data->avplayer pause];
        _status = PAUSED;
    }
}


void OSXAVFoundationVideo::clear()
{
    if (_data)
        _data->clear();
}


void OSXAVFoundationVideo::quit(bool t)
{
    pause();
}


void OSXAVFoundationVideo::seek(double pos)
{
    static CMTime tolerance = CMTimeMakeWithSeconds(0.01, 600);
    if(_data->avplayer)
        [_data->avplayer seekToTime: CMTimeMakeWithSeconds(pos, 600) toleranceBefore:  tolerance toleranceAfter: tolerance];
    requestNewFrame();
}
    
double OSXAVFoundationVideo::getCurrentTime () const
{
    return _data->avplayer ? CMTimeGetSeconds([_data->avplayer currentTime]) : 0;
}

void OSXAVFoundationVideo::open(const std::string& filename)
{
    NSAutoreleasePoolHelper helper;
    
    clear();
    
    _data->delegate = [[OSXAVFoundationVideoDelegate alloc] init];
    _data->delegate.video = this;
    
    NSURL* url(NULL);
    if (osgDB::containsServerAddress(filename))
    {
        url = [NSURL URLWithString: toNSString(filename)];
    }
    else
    {
        url = [NSURL fileURLWithPath: toNSString(filename)];
    }

     _data->output = [[AVPlayerItemVideoOutput alloc] initWithPixelBufferAttributes:
        [NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithInt:kCVPixelFormatType_32BGRA], kCVPixelBufferPixelFormatTypeKey,
        [NSNumber numberWithInteger:1], kCVPixelBufferBytesPerRowAlignmentKey,
        [NSNumber numberWithBool:YES], kCVPixelBufferOpenGLCompatibilityKey,
        nil]];
    
    if (_data->output)
    {
        _data->output.suppressesPlayerRendering = YES;
    }
    
    _data->avplayer = [AVPlayer playerWithURL: url]; // AVPlayerFactory::instance()->getOrCreate(url);
    [_data->avplayer retain];
    
    _data->avplayer.actionAtItemEnd = AVPlayerActionAtItemEndNone;
    
    [_data->avplayer.currentItem addOutput:_data->output];
    
    
    [[NSNotificationCenter defaultCenter] addObserver: _data->delegate
        selector:@selector(playerItemDidReachEnd:)
        name:AVPlayerItemDidPlayToEndTimeNotification
        object:_data->avplayer.currentItem];
    
    
    _videoDuration = CMTimeGetSeconds([_data->avplayer.currentItem duration]);
    
    // get the max size of the video-tracks
    NSArray* tracks = [_data->avplayer.currentItem.asset tracksWithMediaType: AVMediaTypeVideo];
    CGSize size;
    for(unsigned int i=0; i < [tracks count]; ++i)
    {
        AVAssetTrack* track = [tracks objectAtIndex:i];
        size = track.naturalSize;
        _framerate = track.nominalFrameRate;

        CGAffineTransform txf = [track preferredTransform];

        osg::Matrixf mat;
        mat.makeIdentity();

        if(!CGAffineTransformIsIdentity(txf))
        {
            // user should take this into account and transform accordingly...
            mat(0,0) = txf.a;
            mat(1,0) = txf.c;
            mat(3,0) = txf.tx;

            mat(0,1) = txf.b;
            mat(1,1) = txf.d;
            mat(3,1) = txf.ty;
        }

        this->setUserValue("preferredTransform", mat);
    }

    _s = size.width;
    _t = size.height;
    _r = 1;
    unsigned char* buffer = (unsigned char*)calloc(_s*_t*4, 1);
    setImage(_s, _t, 1, GL_RGBA, GL_BGRA, GL_UNSIGNED_BYTE, buffer, USE_MALLOC_FREE);
    
    _fileName = filename;
        
    requestNewFrame();
    
    _status = PAUSED;
    _fileOpened = true;
}
    
float OSXAVFoundationVideo::getVolume() const
{
    return _volume;
}


void OSXAVFoundationVideo::setVolume(float v)
{
    NSAutoreleasePoolHelper helper;
    _volume = v;
    if (_data->avplayer)
        [_data->avplayer setVolume: v];
}


float OSXAVFoundationVideo::getAudioBalance()
{
    return 0.0f;
}


void OSXAVFoundationVideo::setAudioBalance(float b)
{
    OSG_WARN << "OSXAVFoundationVideo: setAudioBalance not supported!" << std::endl;
}

            


    
void OSXAVFoundationVideo::decodeFrame()
{
    // std::cout << this << " decodeFrame: " << _waitForFrame << std::endl;
    
    if (!_fileOpened)
        return;
    
    NSAutoreleasePoolHelper helper;
    
    bool is_valid = (_data && (_data->avplayer.status != AVPlayerStatusFailed));
    if (!is_valid)
    {
        _waitForFrame = false;
        pause();
        OSG_WARN << "OSXAVFoundationVideo: " << toString([_data->avplayer.error localizedFailureReason]) << std::endl;
    }
    
    bool is_playing = is_valid && (getTimeMultiplier() != 0);
    
    CMTime outputItemTime = [_data->output itemTimeForHostTime:CACurrentMediaTime()];
    
    if (_waitForFrame || [_data->output hasNewPixelBufferForItemTime:outputItemTime])
    {
        
        CVPixelBufferRef newframe =  [_data->output copyPixelBufferForItemTime:outputItemTime itemTimeForDisplay:NULL];
        if (newframe)
        {
            if (isCoreVideoUsed())
            {
                CVPixelBufferLockBaseAddress(newframe, kCVPixelBufferLock_ReadOnly);
                int w = CVPixelBufferGetWidth(newframe);
                int h = CVPixelBufferGetHeight(newframe);

                #if (TARGET_OS_IPHONE)
                    CVOpenGLESTextureRef texture = NULL;
                    CVReturn err = CVOpenGLESTextureCacheCreateTextureFromImage(kCFAllocatorDefault, _data->coreVideoTextureCache, newframe, NULL, GL_TEXTURE_2D, GL_RGBA, w, h, GL_BGRA, GL_UNSIGNED_BYTE, 0, &texture);
                    if (err)
                    {
                        OSG_WARN << "OSXAVFoundationVideo :: could not create texture from image, err: " << err << std::endl;
                    }
                    _data->addFrame(texture);
                #else
                    CVOpenGLTextureRef texture = NULL;
                    CVReturn err = CVOpenGLTextureCacheCreateTextureFromImage(kCFAllocatorDefault, _data->coreVideoTextureCache, newframe, 0, &texture);
                    if (err)
                    {
                        OSG_WARN << "OSXAVFoundationVideo :: could not create texture from image, err: " << err << std::endl;
                    }
                            
                    _data->addFrame(texture);
                #endif
                _dimensionsChangedCallbackNeeded = (_s != w) || (_t != h);
                _s = w; _t = h; _r = 1;

                CVPixelBufferUnlockBaseAddress(newframe, kCVPixelBufferLock_ReadOnly);
                CVPixelBufferRelease(newframe);
            }
            else
            {
                _data->addFrame(newframe);
            }
            _waitForFrame = false;
            
        }
    }
    
    _status = is_valid ? is_playing ? PLAYING : PAUSED : INVALID;
}

void OSXAVFoundationVideo::update(osg::NodeVisitor *)
{
    if (!getVideoFrameDispatcher())
        decodeFrame();

    if (isCoreVideoUsed())
    {
        if (_dimensionsChangedCallbackNeeded)
            handleDimensionsChangedCallbacks();
        _dimensionsChangedCallbackNeeded = false;
        
        return;
    }
    
        
    if (_data->hasNewFrame())
    {
        CVPixelBufferRef newframe = _data->getLastFrame();
        
        CVPixelBufferLockBaseAddress(newframe,kCVPixelBufferLock_ReadOnly);
       
        size_t width = CVPixelBufferGetWidth(newframe);
        size_t height = CVPixelBufferGetHeight(newframe);
        size_t bpr = CVPixelBufferGetBytesPerRow(newframe);

        // Get the base address of the pixel buffer
        void *baseAddress = CVPixelBufferGetBaseAddress(newframe);
        setImage(width, height, 1, GL_RGBA, GL_BGRA, GL_UNSIGNED_BYTE, (unsigned char*)baseAddress, NO_DELETE, 1, bpr/4);
        // std::cout << this << " new frame: " << width << "x" << height << " " << baseAddress << std::endl;
        CVPixelBufferUnlockBaseAddress(newframe, kCVPixelBufferLock_ReadOnly);
    }
}


bool OSXAVFoundationVideo::needsDispatching() const
{
    // std::cout << this << " needs dispatching: " << (_waitForFrame || getNeedsDispatching()) << std::endl;
    return  _waitForFrame || getNeedsDispatching();
}



void OSXAVFoundationVideo::applyLoopingMode()
{
    // looping is handled by the delegate
}


void OSXAVFoundationVideo::requestNewFrame()
{
    setNeedsDispatching(RequestSingleUpdate);
    _waitForFrame = true;
}

bool OSXAVFoundationVideo::getCurrentCoreVideoTexture(GLenum& target, GLint& name, int& width, int& height) const
{
    #if (TARGET_OS_IPHONE)
        CVOpenGLESTextureCacheFlush(_data->coreVideoTextureCache, 0);
        CVOpenGLESTextureRef texture = _data->getLastFrame();
        if(texture) {
            target = GL_TEXTURE_2D;
            name = CVOpenGLESTextureGetName(texture);
            width = _s;
            height = _t;
        }
        return (texture != NULL);
    #else
        CVOpenGLTextureCacheFlush(_data->coreVideoTextureCache, 0);
        CVOpenGLTextureRef texture = _data->getLastFrame();
        if (texture)
        {
            target = CVOpenGLTextureGetTarget(texture);
            name = CVOpenGLTextureGetName(texture);
            width = _s;
            height = _t;
        }
        
        return (texture != NULL);
    #endif
}


void OSXAVFoundationVideo::lazyInitCoreVideoTextureCache(osg::State& state)
{
    if (_data->coreVideoTextureCache)
        return;
    #if (TARGET_OS_IPHONE)
        osgViewer::GraphicsWindowIOS* win = dynamic_cast<osgViewer::GraphicsWindowIOS*>(state.getGraphicsContext());
        if (win)
        {
            EAGLContext* context = win->getContext();
            CVReturn err = CVOpenGLESTextureCacheCreate(kCFAllocatorDefault, NULL, context, NULL, &_data->coreVideoTextureCache);
            if (err) 
            {
                 OSG_WARN << "OSXAVFoundationVideo : could not create texture cache :" << err << std::endl;
            }
        }
    
    #else
        osgViewer::GraphicsWindowCocoa* win = dynamic_cast<osgViewer::GraphicsWindowCocoa*>(state.getGraphicsContext());
        if (win)
        {
            NSOpenGLContext* context = win->getContext();
            CGLContextObj cglcntx = (CGLContextObj)[context CGLContextObj];
            CGLPixelFormatObj cglPixelFormat = (CGLPixelFormatObj)[ win->getPixelFormat() CGLPixelFormatObj];
            CVReturn cvRet = CVOpenGLTextureCacheCreate(kCFAllocatorDefault, 0, cglcntx, cglPixelFormat, 0, &_data->coreVideoTextureCache);
            if (cvRet != kCVReturnSuccess)
            {
                OSG_WARN << "OSXAVFoundationVideo : could not create texture cache :" << cvRet << std::endl;
            }
        }
    #endif
}


osg::Texture* OSXAVFoundationVideo::createSuitableTexture()
{
    #if (TARGET_OS_IPHONE)
        return new OSXAVFoundationCoreVideoTexture(this);
    #else
        return NULL; // new OSXAVFoundationCoreVideoTexture(this);
    #endif
}

    
