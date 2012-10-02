#include "OSXAVFoundationVideo.h"

#include <osgdB/FileNameUtils>
#include <osgViewer/api/Cocoa/GraphicsWindowCocoa>
#include <iostream>

#import <AVFoundation/AVFoundation.h>
#import <Cocoa/Cocoa.h>



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
    AVPlayerItem* avplayeritem;
    AVPlayerItemVideoOutput* output;
    OSXAVFoundationVideoDelegate* delegate;
    std::vector<CVBufferRef> lastFrames;
    int readFrameNdx, writeFrameNdx;
    CVOpenGLTextureCacheRef coreVideoTextureCache;
    
    Data()
        : avplayer(NULL)
        , avplayeritem(NULL)
        , output(NULL)
        , delegate(NULL)
        , lastFrames(3)
        , readFrameNdx(0)
        , writeFrameNdx(0)
        , coreVideoTextureCache(0)
    {
    }
    ~Data() {
        [output release];
        [avplayeritem release];
        [avplayer release];
        
        [delegate release];
        
        for(unsigned int i=0; i< lastFrames.size(); ++i)
        {
            if (lastFrames[i])
            {
                CVBufferRelease(lastFrames[i]);
            }
        }
        
        if (coreVideoTextureCache)
        {
            CVOpenGLTextureCacheRelease(coreVideoTextureCache);
            coreVideoTextureCache = NULL;
        }
        output = NULL;
        avplayer = NULL;
        avplayeritem = NULL;
        delegate = NULL;
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
}


OSXAVFoundationVideo::~OSXAVFoundationVideo()
{
    quit();
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
    if (_data->avplayer) {
        [_data->avplayer pause];
        _status = PAUSED;
        setNeedsDispatching(StopUpdate);
    }
}


void OSXAVFoundationVideo::clear()
{
    [_data->output release];
    [_data->avplayeritem release];
    [_data->avplayer release];
    
    if (_data->delegate) {
        [[NSNotificationCenter defaultCenter] removeObserver: _data->delegate
            name:AVPlayerItemDidPlayToEndTimeNotification
            object:[_data->avplayer currentItem]
        ];
    }
    
    [_data->delegate release];
    
    _data->output = NULL;
    _data->avplayer = NULL;
    _data->avplayeritem = NULL;
    _data->delegate = NULL;
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
    
    _data->avplayeritem = [[AVPlayerItem alloc] initWithURL: url];
    _data->avplayer = [AVPlayer playerWithPlayerItem: _data->avplayeritem];
    _data->avplayer.actionAtItemEnd = AVPlayerActionAtItemEndNone;
    
    [[_data->avplayer currentItem] addOutput:_data->output];
    
    [[NSNotificationCenter defaultCenter] addObserver: _data->delegate
        selector:@selector(playerItemDidReachEnd:)
        name:AVPlayerItemDidPlayToEndTimeNotification
        object:[_data->avplayer currentItem]];
    
    _videoDuration = CMTimeGetSeconds([[_data->avplayer currentItem] duration]);
    
    // get the max size of the video-tracks
    NSArray* tracks = [_data->avplayeritem.asset tracksWithMediaType: AVMediaTypeVideo];
    CGSize size;
    for(unsigned int i=0; i < [tracks count]; ++i)
    {
        AVAssetTrack* track = [tracks objectAtIndex:i];
        size = track.naturalSize;
        _framerate = track.nominalFrameRate;
    }
    
    _s = size.width;
    _t = size.height;
    _r = 1;
    
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

                CVOpenGLTextureRef texture = NULL;
                CVReturn err = CVOpenGLTextureCacheCreateTextureFromImage(kCFAllocatorDefault, _data->coreVideoTextureCache, newframe, 0, &texture);
                if (err)
                {
                    OSG_WARN << "OSXAVFoundationVideo :: could not create texture from image, err: " << err << std::endl;
                }
                int w = CVPixelBufferGetWidth(newframe);
                int h = CVPixelBufferGetHeight(newframe);
                _dimensionsChangedCallbackNeeded = (_s != w) || (_t != h);
                _s = w; _t = h; _r = 1;
                
                _data->addFrame(texture);
            
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

        // Get the base address of the pixel buffer
        void *baseAddress = CVPixelBufferGetBaseAddress(newframe);
        setImage(width, height, 1, GL_RGBA, GL_BGRA, GL_UNSIGNED_BYTE, (unsigned char*)baseAddress, NO_DELETE);
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
}


void OSXAVFoundationVideo::lazyInitCoreVideoTextureCache(osg::State& state)
{
    if (_data->coreVideoTextureCache)
        return;
    
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
}
    
