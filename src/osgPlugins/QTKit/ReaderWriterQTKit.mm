// Modern QTKit/Core Video plugin for OSG.
// Eric Wing

#include <osg/ImageStream>
#include <osg/Notify>
#include <osg/Geode>

#include <osgDB/Registry>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>

#import <Foundation/Foundation.h>
#import <QTKit/QTKit.h>

// Optimization to share the CoreVideo pixelbuffer with the ImageStream data avoiding memcpy's.
// Risks are that we are more exposed to race conditions when we update the image
// since Core Video updates happen on a background thread.
#define SHARE_CVPIXELBUFFER 1

/* Implementation notes:
 * The first problem is that the whole OSG design is centered around pumping
 * osg::ImageStreams through the system.
 * But Core Video actual can give us OpenGL textures that are ready-to-go.
 * (Core Video can also give us PBO's, but there seems to be an issue elsewhere
 * in OSG w.r.t. PBOs on OS X.)
 * OSG is not friendly with dealing with textures created from the outside.
 * In the interests of getting something working (but not optimal),
 * I use the Core Video PixelBuffer APIs to fetch data to main memory to provide osg::ImageStream
 * with data let it upload the texture data. What a waste!
 *
 * The second problem is that Apple still hasn't updated their QTKit/Core Video glue API
 * to be 64-bit ready as of Snow Leopard. 
 * This means that this plugin only works in 32-bit until Apple gets their act together.
 * We also can't activate the Quicktime X optimized playback renderer.
 *
 * The third problem is that OSG makes some really bad assumptions about when the movie size (width,height)
 * becomes available. Particularly with live streams, movie sizes are allowed to change inflight and 
 * you may not get a valid movie size until after you initially start playing the stream. OSG assumes
 * you have the correct movie size right on open and that it will never change.
 *
 * The forth problem is that there are so many plugins competing for the same movie types
 * I don't know what's going on and it seems rather rigid being a compile time things we must set.
 *
 * Also note for audio, this plugin completely ignores/bypasses the AudioStream class.
 */

static CVReturn MyDisplayLinkCallback(CVDisplayLinkRef display_link, 
                               const CVTimeStamp* in_now,
                               const CVTimeStamp* in_output_time,
                               CVOptionFlags flags_in,
                               CVOptionFlags* flags_out,
                               void* user_data);
//@class MovieNotificationHandler;

namespace osgQTKit
{
    class QTKitImageStream;
}

@interface MovieNotificationHandler : NSObject
{
    osgQTKit::QTKitImageStream* imageStream;
}

- (void) setImageStream:(osgQTKit::QTKitImageStream*)image_stream;
- (void) movieNaturalSizeDidChange:(NSNotification*)the_notification;
- (void) movieLoadStateDidChange:(NSNotification*)the_notification;
- (void) movieDidEnd:(NSNotification*)the_notification;

@end

    
namespace osgQTKit
{

class QTKitImageStream : public osg::ImageStream
{
    public:
        QTKitImageStream():
        ImageStream(),
            displayLink(NULL),
#if SHARE_CVPIXELBUFFER
            currentSwapFrameIndex(0),
#else
            currentFrame(NULL),
#endif
            qtMovie(nil),
            pixelBufferContextForQTOpenGL(NULL)
        {
#if SHARE_CVPIXELBUFFER
            swapFrame[0] = NULL;
            swapFrame[1] = NULL;
#endif
               setOrigin(osg::Image::TOP_LEFT);
            initDisplayLink();
//            Class movie_notification_class = NSClassFromString(@"MovieNotificationHandler");
//            movieNotificationHandler = [[movie_notification_class alloc] init];
            movieNotificationHandler = [[MovieNotificationHandler alloc] init];
            [movieNotificationHandler setImageStream:this];
            // for optional garbage collection dancing
            CFRetain(movieNotificationHandler);
            [movieNotificationHandler release];
        }

        /** Copy constructor using CopyOp to manage deep vs shallow copy. */
        QTKitImageStream(const QTKitImageStream& image,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY):
            ImageStream(image,copyop) {}

        // Core Video requires the CGLContent and CGLPixelFormat
        // to setup the displaylink

        void initDisplayLink()
        {
            NSAutoreleasePool* autorelease_pool = [[NSAutoreleasePool alloc] init];
            if (NULL != displayLink)
            {
                if(CVDisplayLinkIsRunning(displayLink))
                {
                    CVDisplayLinkStop(displayLink);
                }
                CVDisplayLinkRelease(displayLink);
                displayLink = NULL;
            }

            // Because we don't have easy access to CGDisplayIDs, we create a displaylink which
            // will work with all the active displays. This is the most flexible option, though maybe
            // not always the fastest.
            CVDisplayLinkCreateWithActiveCGDisplays(&displayLink);
            if (NULL != displayLink)
            {
                // set the renderer output callback function
                CVDisplayLinkSetOutputCallback(displayLink, &MyDisplayLinkCallback, this);
            }
    
            [autorelease_pool drain];
        }

        META_Object(osgQTKit,QTKitImageStream);

        void setVolume(float the_volume)
        {
            [qtMovie setVolume:the_volume];
        }

        float getVolume() const
        {
            return [qtMovie volume];
        }

        bool open(const std::string& file_name)
        {
            NSAutoreleasePool* autorelease_pool = [[NSAutoreleasePool alloc] init];
            NSString* ns_string = [NSString stringWithUTF8String:file_name.c_str()];
            NSError* the_error = nil;

            if(nil != qtMovie)
            {
                // Do we allow this?
                // Don't return without cleaning up autorelease pool
                // shutdown displaylink if this is allowed
                //        CFRelease(objCData->qtMovie);
                [autorelease_pool drain];
                return false;
            }


            // Use QTMovieOpenForPlaybackAttribute to activate Quicktime X
            // (Disabled because we can't use this while some APIs we use are 32-bit)
            NSDictionary* movie_attributes =
                [NSDictionary dictionaryWithObjectsAndKeys:
                ns_string, QTMovieFileNameAttribute,
//                [NSNumber numberWithBool:YES], QTMovieLoopsAttribute,
//                [NSNumber numberWithBool:YES], QTMovieOpenForPlaybackAttribute,
                nil];
            qtMovie = [[QTMovie alloc] initWithAttributes:movie_attributes error:&the_error];
            if(nil != qtMovie)
            {
                // For garbage collection, we need to make sure to hold the reference
                // This code is designed to work for both modes.
                CFRetain(qtMovie);
                [qtMovie release];
            }
            else
            {
//                NSLog(@"Failed to open file: %@, %@", ns_string, [the_error localizedDescription]);
                OSG_WARN<<"Failed to open file: " << file_name << std::endl;

            }
    
            [[NSNotificationCenter defaultCenter] addObserver:movieNotificationHandler
                selector:@selector(movieNaturalSizeDidChange:)
#if defined(MAC_OS_X_VERSION_10_6) && (MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_6)
                name:QTMovieNaturalSizeDidChangeNotification
#else
                name:QTMovieSizeDidChangeNotification
#endif    
                object:qtMovie];

            [[NSNotificationCenter defaultCenter] addObserver:movieNotificationHandler
                selector:@selector(movieLoadStateDidChange:)
                name:QTMovieLoadStateDidChangeNotification
                object:qtMovie];

            [[NSNotificationCenter defaultCenter] addObserver:movieNotificationHandler
                selector:@selector(movieDidEnd:)
                name:QTMovieDidEndNotification
                object:qtMovie];

            [autorelease_pool drain];
            return true;


        }


        virtual void play()
        {
            NSAutoreleasePool* autorelease_pool = [[NSAutoreleasePool alloc] init];

            _status=PLAYING;
            if(NULL == pixelBufferContextForQTOpenGL)
            {
                // This isn't guaranteed to yield a valid size.
                // We are supposed to wait for a callback after the video stream connection has been established.
                NSSize movie_size = [[qtMovie attributeForKey:QTMovieNaturalSizeAttribute] sizeValue];
//                NSLog(@"movie_size=%f, %f", movie_size.width, movie_size.height);
                NSDictionary* pixel_buffer_attributes =
                    [NSDictionary dictionaryWithObjectsAndKeys:
#if __BIG_ENDIAN__
                        [NSNumber numberWithInteger:k32ARGBPixelFormat], kCVPixelBufferPixelFormatTypeKey,
#else
                        [NSNumber numberWithInteger:k32BGRAPixelFormat], kCVPixelBufferPixelFormatTypeKey,
#endif    
                        // Seems that Core Video will figure out the size automatically.
                        // Probably better that way since our values may be wrong.
//                        [NSNumber numberWithFloat:movie_size.width], kCVPixelBufferWidthKey,
//                        [NSNumber numberWithFloat:movie_size.height], kCVPixelBufferHeightKey,
                        [NSNumber numberWithInteger:1], kCVPixelBufferBytesPerRowAlignmentKey,
                        [NSNumber numberWithBool:YES], kCVPixelBufferOpenGLCompatibilityKey,
                        nil
                     ];
                NSDictionary* visual_context_options =
                    [NSDictionary dictionaryWithObjectsAndKeys:
                        pixel_buffer_attributes, kQTVisualContextPixelBufferAttributesKey,
                        nil
                    ];

                OSStatus the_error = QTPixelBufferContextCreate(
                    kCFAllocatorDefault,                                        // an allocator to Create functions
                    (CFDictionaryRef)visual_context_options,                                                        // a CF Dictionary of attributes
                    &pixelBufferContextForQTOpenGL);
                if(noErr != the_error)
                {
                    NSLog(@"Error calling QTPixelBufferContextCreate: os_status=%d, pixelBufferContextForQTOpenGL=%x", the_error, pixelBufferContextForQTOpenGL);
    
                }
                // Because of bad osgmovie assumptions, we need to set the size now even though we may not have correct information.
#if SHARE_CVPIXELBUFFER
                setImage((int)movie_size.width,(int)movie_size.height,1,
                         GL_RGBA8,
                         GL_BGRA,
                         GL_UNSIGNED_INT_8_8_8_8_REV,
                         NULL,
                         osg::Image::NO_DELETE,
                         1);
#else
                allocateImage((int)movie_size.width,(int)movie_size.height,1,GL_BGRA,GL_UNSIGNED_INT_8_8_8_8_REV,1);
                setInternalTextureFormat(GL_RGBA8);
#endif
    
                SetMovieVisualContext([qtMovie quickTimeMovie], pixelBufferContextForQTOpenGL);

            }
    
    

            if(!CVDisplayLinkIsRunning(displayLink))
            {
                CVReturn err_flag = CVDisplayLinkStart(displayLink);
                if(kCVReturnSuccess != err_flag)
                {
                    NSLog(@"Error CVDisplayLinkStart()");
                }


                [qtMovie play];
            }
            else
            {
//                NSLog(@"Alreadying playing");
            }

    
    
            [autorelease_pool drain];

        }

        virtual void pause()
        {
            NSAutoreleasePool* autorelease_pool = [[NSAutoreleasePool alloc] init];
            _status=PAUSED;
            if(CVDisplayLinkIsRunning(displayLink))
            {
                CVDisplayLinkStop(displayLink);
                [qtMovie stop];
            }
            [autorelease_pool drain];
        }

        virtual void rewind()
        {
            NSAutoreleasePool* autorelease_pool = [[NSAutoreleasePool alloc] init];
            _status=REWINDING; // seriously? This means that the movie will continue to be in this state until played/paused
            [qtMovie gotoBeginning];
            [autorelease_pool drain];
        }
    
        // OSG Documentation doesn't say what time is.
        // Is it an absolute time in the movie, or an offset to move based on the current time (which can be negative)?
        // And what are the units? seconds? milliseconds, minutes?
        virtual void seek(double seek_time)
        {
    
        /*
        http://developer.apple.com/mac/library/technotes/tn2005/tn2138.html
          QTTime oldTime = [qtMovie currentTime];
         QTTime incTime = QTTimeFromString( @"00:02:00.00" );
         QTTime newTime = QTTimeIncrement( oldTime, incTime );
    
         NSLog( QTStringFromTime( oldTime ) );
         NSLog( QTStringFromTime( incTime ) );
         NSLog( QTStringFromTime( newtime ) );
         I get the following results:
    
         0:00:00:00.00/48000
         0:00:00:00.00/1000000
         0:00:00:00.00/1000000
         I have also tried setting the time string to @"0:00:02:00.00", @"0:0:2:0.0", and other variations. No luck.
    
         What am I doing wrong?
    
         A: You'll notice the following comment in QTTime.h:
    
         // ,,,dd:hh:mm:ss.ff/ts
         which translates into:
    
         days:hours:minutes:seconds:frames/timescale
         So you should try a string like:
    
         QTTime incTime = QTTimeFromString( @"00:00:02:00.00/600" );
         NSLog( QTStringFromTime( incTime ) );
         */
    
        }

        virtual void quit()
        {
            close();
        }

        virtual void setTimeMultiplier(double the_rate)
        {
            NSAutoreleasePool* autorelease_pool = [[NSAutoreleasePool alloc] init];
            [qtMovie setRate:the_rate];
            [autorelease_pool drain];
        }
        virtual double getTimeMultiplier() const
        {
            NSAutoreleasePool* autorelease_pool = [[NSAutoreleasePool alloc] init];
            float the_rate = [qtMovie rate];
            [autorelease_pool drain];
            return the_rate;
        }

        CVReturn handleCoreVideoCallback(
            CVDisplayLinkRef display_link,
            const CVTimeStamp* in_now,
            const CVTimeStamp* in_output_time,
            CVOptionFlags flags_in,
            CVOptionFlags* flags_out,
            void* user_data
        )
        {
//            NSLog(@"In handleCoreVideoCallback");
            if(NULL == pixelBufferContextForQTOpenGL)
            {
//                NSLog(@"pixelBufferContextForQTOpenGL is NULL");
                return kCVReturnSuccess;
            }
            // CoreVideo callbacks happen on a secondary thread.
            // So we need a new autorelease pool for this thread.
            NSAutoreleasePool* autorelease_pool = [[NSAutoreleasePool alloc] init];
            // check for new frame

    
            /*
             * Notes: The SHARE_CVPIXELBUFFER stuff reuses the same memory allocated by Core Video
             * for the osg::Image data. This avoids extra memcpy's.
             * FIXME: This probably needs locking. What is the locking model for osg::Image?
             * Since Core Video operates on a high priority background thread, it is possible
             * that the osg::Image could be utilized while we are updating with new frame data.
             * Experimentally, I have not gotten any crashes, but I have noticed flickering
             * in my original implementation where I would first set the osg::Image data to NULL
             * before releasing the CVPixelBuffer. To avoid the flickering, I have now implemented
             * a double-buffering technique where I immediately provide the new data and then
             * clean up after the swap.
             */
            if(QTVisualContextIsNewImageAvailable(pixelBufferContextForQTOpenGL, in_output_time))
            {
#if SHARE_CVPIXELBUFFER
                size_t previous_swap_frame_index = currentSwapFrameIndex;

                // flip the active swap buffer
                if(0 == currentSwapFrameIndex)
                {
                    currentSwapFrameIndex = 1;
                }
                else
                {
                    currentSwapFrameIndex = 0;
                }

                OSStatus error_status = QTVisualContextCopyImageForTime(pixelBufferContextForQTOpenGL, NULL, in_output_time, &swapFrame[currentSwapFrameIndex]);
                // the above call may produce a null frame so check for this first
                // if we have a frame, then draw it
                if ((noErr == error_status) && (NULL != swapFrame[currentSwapFrameIndex]))
                {
                    size_t buffer_width = CVPixelBufferGetWidth(swapFrame[currentSwapFrameIndex]);
                    size_t buffer_height = CVPixelBufferGetHeight(swapFrame[currentSwapFrameIndex]);
//                                        NSLog(@"CVPixelBuffer w=%d, h=%d", buffer_width, buffer_height);
                    //                    buffer_width = 480;
                    //                    buffer_height = 320;

#if defined(MAC_OS_X_VERSION_10_6) && (MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_6)
                    CVPixelBufferLockBaseAddress( swapFrame[currentSwapFrameIndex], kCVPixelBufferLock_ReadOnly );
#else
                    CVPixelBufferLockBaseAddress( swapFrame[currentSwapFrameIndex], 0 );
#endif  
                    void* raw_pixel_data = CVPixelBufferGetBaseAddress(swapFrame[currentSwapFrameIndex]);
    
                    setImage(buffer_width,buffer_height,1,
                             GL_RGBA8,
                             GL_BGRA,
                             GL_UNSIGNED_INT_8_8_8_8_REV,
                             (unsigned char *)raw_pixel_data,
                             osg::Image::NO_DELETE,
                             1);
                    // seems to have no effect. Flip image the hard way
                    //                    setOrigin(osg::Image::TOP_LEFT);
                    //    flipVertical();
    
    
                    CVPixelBufferUnlockBaseAddress( swapFrame[currentSwapFrameIndex], 0 );
                }
    
                // Now clean up previous frame
                // Release the previous frame. (This is safe to call even if it is NULL.)
                CVPixelBufferRelease(swapFrame[previous_swap_frame_index]);
                swapFrame[previous_swap_frame_index] = NULL;
    
#else
                // if we have a previous frame release it
                if (NULL != currentFrame)
                {
                    CVPixelBufferRelease(currentFrame);
                    currentFrame = NULL;
                }

                // get a "frame" (image buffer) from the Visual Context, indexed by the provided time
                OSStatus error_status = QTVisualContextCopyImageForTime(pixelBufferContextForQTOpenGL, NULL, in_output_time, &currentFrame);
                // the above call may produce a null frame so check for this first
                // if we have a frame, then draw it
                if ((noErr == error_status) && (NULL != currentFrame))
                {
                    size_t buffer_width = CVPixelBufferGetWidth(currentFrame);
                    size_t buffer_height = CVPixelBufferGetHeight(currentFrame);
                    //                    NSLog(@"CVPixelBuffer w=%d, h=%d", buffer_width, buffer_height);
                    //                    buffer_width = 480;
                    //                    buffer_height = 320;
    
                    CVPixelBufferLockBaseAddress( currentFrame, kCVPixelBufferLock_ReadOnly );
    
                    void* raw_pixel_data = CVPixelBufferGetBaseAddress(currentFrame);
    
    
                    /*
                     NSLog(@"CVPixelBufferGetDataSize(currentFrame)=%d", CVPixelBufferGetDataSize(currentFrame));
                     NSLog(@"CVPixelBufferIsPlanar(currentFrame)=%d", CVPixelBufferIsPlanar(currentFrame));
                     NSLog(@"CVPixelBufferGetBytesPerRow(currentFrame)=%d", CVPixelBufferGetBytesPerRow(currentFrame));
                     */
    
                    // Don't understand why CVPixelBufferGetDataSize returns a slightly bigger size
                    // e.g. for a 480x320 movie, it is 32-bytes larger than 480*320*4
                    //                    memcpy(data(),raw_pixel_data,CVPixelBufferGetDataSize(currentFrame));
                    memcpy(data(),raw_pixel_data,buffer_width*buffer_height*4);
    
                    //                    flipVertical();
                    dirty();
                    CVPixelBufferUnlockBaseAddress( currentFrame, 0 );
                }
#endif
            } // end QTVisualContextIsNewImageAvailable()
    

            [autorelease_pool drain];
            return kCVReturnSuccess;
        }

        // TODO: OSG really needs some kind of notification callback for this so your OSG can find out that
        // the movie size has changed.
        void handleMovieNaturalSizeDidChange()
        {
//            NSSize movie_size = [[qtMovie attributeForKey:QTMovieNaturalSizeAttribute] sizeValue];
//            NSLog(@"handleMovieNaturalSizeDidChange=%f, %f", movie_size.width, movie_size.height);
            pause();
            QTVisualContextRelease(pixelBufferContextForQTOpenGL);
            pixelBufferContextForQTOpenGL = NULL;
            play();
        }

        // Untested: I think the important case to handle is usually live streaming and there is underrun.
        void handleMovieLoadStateDidChange()
        {
//            NSLog(@"handleMovieLoadStateDidChange");
            if( (PLAYING == _status) && ([qtMovie rate] == 0.0) ) // if should be playing, but not playing
            {
//                NSLog(@"not playing");
                if([[qtMovie attributeForKey:QTMovieLoadStateAttribute] longValue] >= kMovieLoadStatePlaythroughOK)
                {
//                    NSLog(@"handleMovieLoadStateDidChangeCallback play");
    
                    [qtMovie play];
                }
            }
            else
            {
//                NSLog(@"playing");
            }
        }

        void handleMovieDidEnd()
        {
            pause();
            QTVisualContextRelease(pixelBufferContextForQTOpenGL);
            pixelBufferContextForQTOpenGL = NULL;
            // should I rewind? What is the expected behavior?
        }
    
    protected:
        CVDisplayLinkRef displayLink;
#if SHARE_CVPIXELBUFFER
        CVPixelBufferRef swapFrame[2];
        size_t currentSwapFrameIndex;
#else
        CVPixelBufferRef currentFrame;
#endif
        QTMovie* qtMovie;
        QTVisualContextRef pixelBufferContextForQTOpenGL;
//        id movieNotificationHandler;
        MovieNotificationHandler* movieNotificationHandler;
    
        virtual ~QTKitImageStream()
        {
            close();
            if (NULL != displayLink)
            {
                if(CVDisplayLinkIsRunning(displayLink))
                {
                    CVDisplayLinkStop(displayLink);
                }
                CVDisplayLinkRelease(displayLink);
                displayLink = NULL;
            }
            CFRelease(movieNotificationHandler);
            movieNotificationHandler = nil;
        }

        void close()
        {
            [[NSNotificationCenter defaultCenter] removeObserver:movieNotificationHandler
                name:QTMovieLoadStateDidChangeNotification object:qtMovie];
            
            [[NSNotificationCenter defaultCenter] removeObserver:movieNotificationHandler
#if defined(MAC_OS_X_VERSION_10_6) && (MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_6)
                name:QTMovieNaturalSizeDidChangeNotification
#else
                name:QTMovieSizeDidChangeNotification
#endif    
            
                object:qtMovie];
            
            [[NSNotificationCenter defaultCenter] removeObserver:movieNotificationHandler
                name:QTMovieDidEndNotification object:qtMovie];

            if(CVDisplayLinkIsRunning(displayLink))
            {
                CVDisplayLinkStop(displayLink);
            }

            QTVisualContextRelease(pixelBufferContextForQTOpenGL);
            pixelBufferContextForQTOpenGL = NULL;
#if SHARE_CVPIXELBUFFER
            CVPixelBufferRelease(swapFrame[0]);
            swapFrame[0] = NULL;
            CVPixelBufferRelease(swapFrame[1]);
            swapFrame[1] = NULL;
            currentSwapFrameIndex = 0;
#else    
            CVPixelBufferRelease(currentFrame);
            currentFrame = NULL;
#endif

            if(nil != qtMovie)
            {
                CFRelease(qtMovie);
                qtMovie = nil;
            }
        }
        virtual void applyLoopingMode()
        {
            if(NO_LOOPING == _loopingMode)
            {
                [qtMovie setAttribute:[NSNumber numberWithBool:NO] forKey:QTMovieLoopsAttribute];
            }
            else
            {
                [qtMovie setAttribute:[NSNumber numberWithBool:YES] forKey:QTMovieLoopsAttribute];
            }
        }



};


}

static CVReturn MyDisplayLinkCallback(CVDisplayLinkRef display_link, 
                               const CVTimeStamp* in_now,
                               const CVTimeStamp* in_output_time,
                               CVOptionFlags flags_in,
                               CVOptionFlags* flags_out,
                               void* user_data)
{
    if(NULL != user_data)
    {
        osgQTKit::QTKitImageStream* qtkit_image_stream = reinterpret_cast<osgQTKit::QTKitImageStream*>(user_data);
        return qtkit_image_stream->handleCoreVideoCallback(display_link, in_now, in_output_time, flags_in, flags_out, NULL);
    }
    return kCVReturnSuccess;
}

class ReaderWriterQTKit : public osgDB::ReaderWriter
{
    public:

        ReaderWriterQTKit()
        {
            supportsExtension("mov","Quicktime movie format");
            supportsExtension("mpg","Mpeg movie format");
            supportsExtension("mp4","Mpeg movie format");
            supportsExtension("mpv","Mpeg movie format");
            supportsExtension("mpeg","Mpeg movie format");

            // only with Perian
            supportsExtension("avi","");
            supportsExtension("xvid","");
            // only with Flip4Mac
            supportsExtension("wmv","");
            
        }
        virtual bool acceptsExtension(const std::string& extension) const
        {
            return
                osgDB::equalCaseInsensitive(extension,"mov") ||
                osgDB::equalCaseInsensitive(extension,"mpg") ||
                osgDB::equalCaseInsensitive(extension,"mp4") ||
                osgDB::equalCaseInsensitive(extension,"mpv") ||
                osgDB::equalCaseInsensitive(extension,"mpeg") ||
                osgDB::equalCaseInsensitive(extension,"avi") ||
                osgDB::equalCaseInsensitive(extension,"xvid") ||
                osgDB::equalCaseInsensitive(extension,"wmv");
    
        }
    
        virtual ~ReaderWriterQTKit()
        {
            OSG_INFO<<"~ReaderWriterQTKit()"<<std::endl;
        }
        
        virtual const char* className() const { return "QTKit ImageStream Reader"; }

        virtual ReadResult readImage(const std::string& file, const osgDB::ReaderWriter::Options* options) const
        {
            std::string ext = osgDB::getLowerCaseFileExtension(file);
            if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;
            
            std::string fileName;
            if (ext=="QTKit")
            {
                fileName = osgDB::findDataFile( osgDB::getNameLessExtension(file), options);
                OSG_INFO<<"QTKit stipped filename = "<<fileName<<std::endl;
            }
            else
            {
                fileName = osgDB::findDataFile( file, options );
                if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;
            }

            OSG_INFO<<"ReaderWriterQTKit::readImage "<< file<< std::endl;

            osg::ref_ptr<osgQTKit::QTKitImageStream> imageStream = new osgQTKit::QTKitImageStream();

            if (!imageStream->open(fileName)) return ReadResult::FILE_NOT_HANDLED;

            return imageStream.release();
        }

    protected:


};


@implementation MovieNotificationHandler

- (void) setImageStream:(osgQTKit::QTKitImageStream*)image_stream
{
    imageStream = image_stream;
}

// I need to verify if this is being called back on a non-main thread.
// My initial observation led me to think it was being called on a background thread,
// but it could be that I was just confused which thread was the main thread since
// CoreVideo was doing a bunch of stuff on its own background thread.
- (void) movieNaturalSizeDidChange:(NSNotification*)the_notification
{
    imageStream->handleMovieNaturalSizeDidChange();
}

- (void) movieLoadStateDidChange:(NSNotification*)the_notification
{
    imageStream->handleMovieLoadStateDidChange();
}

- (void) movieDidEnd:(NSNotification*)the_notification
{
    imageStream->handleMovieDidEnd();
}
@end


// now register with Registry to instantiate the above
// reader/writer.
REGISTER_OSGPLUGIN(QTKit, ReaderWriterQTKit)
