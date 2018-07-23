/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2007 Robert Osfield
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

#include <cstdlib>

#include <osg/Notify>
#include <osg/Image>
#include <osg/Timer>
#include <osg/ref_ptr>
#include <osg/Referenced>
#include <osg/Notify>
#include <osgDB/Registry>
#include <osg/GL>
#include <osg/Endian>
#include <osg/Timer>
#include <osgDB/FileNameUtils>

#include <OpenThreads/ScopedLock>
#include <OpenThreads/Thread>

#include "QuicktimeLiveImageStream.h"
#include "QTLiveUtils.h"

// Constructor: setup and start thread
QuicktimeLiveImageStream::QuicktimeLiveImageStream(std::string fileName) : ImageStream()
{
    setOrigin(osg::Image::TOP_LEFT);
    _status = ImageStream::PAUSED;

    //probe_video_digitizer_components();
    //probe_sequence_grabber_components();

    // Initialise QT
    //    initialize_quicktime_qtml();
    //    enter_quicktime_movies();
    //
    load(fileName);
}


// Deconstructor: stop and terminate thread
QuicktimeLiveImageStream::~QuicktimeLiveImageStream()
{
    // Terminate QT
    //    leave_quicktime_movies();
    //    terminite_quicktime_qtml();
}

/// Start or continue stream.
void QuicktimeLiveImageStream::play()
{
   OSG_DEBUG<<"Sending play"<<this<<std::endl;
  /* if (g_s_use_sg)
   {
       ComponentResult result = noErr;
       result = SGStartPreview(m_gSeqGrabber);
       if (result != noErr)
           OSG_FATAL << "SGStartPreview : error" << std::endl;
   }*/
}
/// Pause stream at current position.
void QuicktimeLiveImageStream::pause()
{
   OSG_DEBUG<<"Sending pause"<<this<<std::endl;
}
/// stop playing
void QuicktimeLiveImageStream::quit(bool wiatForThreadToExit)
{
   OSG_DEBUG<<"Sending quit"<<this<<std::endl;
}

//
// PRIVATE
//

// Use the Sequence Grabber or the raw Video Digitizer
// If using SG then use it in Preview or Record option
// Thre options - VD Play Through, SG Preview or SG Record
static bool g_s_use_sg        = true ; // 1a
static bool g_s_use_sg_record = false; // 1b

// load
void QuicktimeLiveImageStream::load(std::string fileName)
{
   OSG_DEBUG<<"QuicktimeLive Loading..."<<this<<std::endl;
   // CreateAndRunWithSequenceGrabber
   if (g_s_use_sg)
       createAndRunWithSequenceGrabber(fileName);
   else
       createAndRunWithVideoDigitizer(fileName);
}

// Create the Image
void QuicktimeLiveImageStream::createImage()
{
    // Old
    // char* pointer = (char*)malloc(4 * m_videoRectWidth*m_videoRectHeight + 32);
    // void* buffer  = (void*)(((unsigned long)(pointer + 31) >> 5) << 5);
    // New
    int* buffer = new int [m_videoRectWidth*m_videoRectHeight]; // 1024*1024*4 bytes (32bit RGBA)
    //
    GLenum internalFormat  = (osg::getCpuByteOrder()==osg::BigEndian)?
                              GL_UNSIGNED_INT_8_8_8_8_REV :
                              GL_UNSIGNED_INT_8_8_8_8;

    setImage(m_videoRectWidth,m_videoRectHeight,1,
            (GLint) GL_RGBA8, (GLenum)GL_BGRA, internalFormat,
            (unsigned char*)buffer,osg::Image::NO_DELETE,4);
}

// Create the offscreen GWorld (using Image  as target memory)
void QuicktimeLiveImageStream::createGWorld()
{
    Rect         destinationBounds;
    OSStatus     err;
    GDHandle     origDevice;
    CGrafPtr     origPort;
    destinationBounds.left   = 0;
    destinationBounds.top    = 0;
    destinationBounds.right  = m_videoRectWidth;
    destinationBounds.bottom = m_videoRectHeight;
    err = QTNewGWorldFromPtr(&m_gw, k32ARGBPixelFormat, &destinationBounds,
                             NULL, NULL, 0, (Ptr)data(), 4*m_videoRectWidth);
    if (err !=0 )
    {
        OSG_DEBUG << "Could not create gWorld" << std::endl;
    }
    else
    {
        // Query
        GetGWorld (&origPort, &origDevice);
        SetGWorld (m_gw, NULL); // set current graphics port to offscreen
        m_pixmap = GetGWorldPixMap(m_gw);
        if (m_pixmap)
         {
             if (!LockPixels (m_pixmap)) // lock offscreen pixel map
             {
                 OSG_FATAL << "Could not lock PixMap" << std::endl;
             }
         }
        // Set back
        SetGWorld(origPort, origDevice);
    }
}

// 1.
// CreateAndRunWithSequenceGrabber
void QuicktimeLiveImageStream::createAndRunWithSequenceGrabber(std::string fileName)
{
   std::string::size_type idx = fileName.find(':');
   if (idx == std::string::npos)
   {
       OSG_FATAL << "Error while parsing deviceID:deviceInputID.live path : " << fileName << std::endl;
   }
   // Better c++ code is to use istrstream
   std::string deviceIDStr      = fileName.substr(0,idx);
   std::string deviceInputIDStr = fileName.substr(idx+1);
   m_videoDeviceID      = static_cast<short>(atoi(deviceIDStr.c_str()));
   m_videoDeviceInputID = static_cast<short>(atoi(deviceInputIDStr.c_str()));
   // Get Video Digitizer Rectangle bounds from a Sequence Grabber proxy (using IDs)
   get_video_device_bounds_idstr(m_videoDeviceID, m_videoDeviceInputID, m_videoRectWidth, m_videoRectHeight, m_videoDeviceIDStr);
   // Sound
   m_soundDeviceID = 2; m_soundDeviceInputID = 0;
   //get_sound_device_idstr(m_soundDeviceID, m_soundDeviceInputID, m_soundDeviceIDStr);
   // Create the Image
   createImage();
   // Create the offscreen GWorld (using Image  as target memory)
   createGWorld();
   // Create the Sequence Grabber (using GWorld as target memory)
   createSequenceGrabber();
   // Create the Sequence Grabber Video Channel
   createSequenceGrabberVideoChannel();
   if (g_s_use_sg_record)
   {
       // Create the Sequence Grabber DataProc setup for Record
       createSequenceGrabberDataProc();
   }
   // Create the Sequence Grabber Audio Channel
   createSequenceGrabberAudioChannel();
   // Start the engine Jack!
   // Callbacks
   createSequenceGrabberVideoBottlenecks();

   ComponentResult result = noErr;
   result = SGPrepare( m_gSeqGrabber, TRUE, FALSE);
   if (result != noErr)
   {
       OSG_FATAL << "SGPrepare : error" << std::endl;
   }

   if (g_s_use_sg_record)
   {
       result = SGStartRecord(m_gSeqGrabber);
       if (result != noErr)
       {
           OSG_FATAL << "SGStartRecord : error" << std::endl;
       }
   }
   else
   {
       result = SGStartPreview(m_gSeqGrabber);
       if (result != noErr)
       {
           OSG_FATAL << "SGStartPreview : error" << std::endl;
       }
   }

   _status = ImageStream::PLAYING;
   // Ticker
   start();
}


// 1.
// Create the Sequence Grabber (using GWorld as target memory)
void QuicktimeLiveImageStream::createSequenceGrabber()
{
    ComponentDescription sg_component_description;
    sg_component_description.componentType         = SeqGrabComponentType; /* A unique 4-byte code identifying the command set */
    sg_component_description.componentSubType      = 0L;      /* Particular flavor of this instance */
    sg_component_description.componentManufacturer = 'appl';  /* Vendor identification */
    sg_component_description.componentFlags        = 0L;      /* 8 each for Component,Type,SubType,Manuf/revision */
    sg_component_description.componentFlagsMask    = 0L;      /* Mask for specifying which flags to consider in search, zero during registration */
    long num_sg_components = CountComponents (&sg_component_description);
    if (num_sg_components)
    {
        Component aComponent = 0;
        ComponentDescription full_sg_component_description = sg_component_description;
        aComponent = FindNextComponent(aComponent, &full_sg_component_description);
        if (aComponent)
        {
            m_gSeqGrabber = OpenComponent(aComponent);
            // If we got a sequence grabber, set it up
            if (m_gSeqGrabber != 0L)
            {
                // Check capability and setting of Sequence Grabber
                GDHandle origDevice;
                CGrafPtr origPort;
                // Create GWorld
                GetGWorld (&origPort, &origDevice);
                SetGWorld (m_gw, NULL); // set current graphics port to offscreen
                // Initialize the sequence grabber
                ComponentResult result = noErr;
                result = SGInitialize (m_gSeqGrabber);
                if (result == noErr)
                {
                    // Set GWorld
                    result = SGSetGWorld(m_gSeqGrabber, (CGrafPtr)m_gw, 0);
                    if (result != noErr)
                    {
                        OSG_FATAL << "Could not set GWorld on SG" << std::endl;
                    }
                }
                // Set GWorld back
                SetGWorld(origPort, origDevice);
            }
        }
    }
}

// Create the Sequence Grabber Video Channel
void QuicktimeLiveImageStream::createSequenceGrabberVideoChannel()
{
    // Check capability and setting of Sequence Grabber
    GDHandle origDevice;
    CGrafPtr origPort;
    // Create GWorld
    GetGWorld (&origPort, &origDevice);
    SetGWorld (m_gw, NULL); // set current graphics port to offscreen
    // Setup
    // Get a video channel
    ComponentResult result = SGNewChannel (m_gSeqGrabber, VideoMediaType, &m_gVideoChannel);
    if ((m_gVideoChannel != nil) && (result == noErr))
    {
        result = SGInitChannel(m_gVideoChannel, m_gSeqGrabber);
        Rect gActiveVideoRect;
        // Usage
        if (g_s_use_sg_record)
            result = SGSetChannelUsage (m_gVideoChannel, seqGrabRecord | seqGrabLowLatencyCapture);
        else
        {
            result = SGSetChannelUsage (m_gVideoChannel, seqGrabPreview);
        }
        //  result = SGSetUseScreenBuffer(m_gVideoChannel, FALSE);
        // Set
        OSG_DEBUG << "Setting up vdig from input prefs" << std::endl;
        result = SGSetChannelDevice     ( m_gVideoChannel, m_videoDeviceIDStr);
        result = SGSetChannelDeviceInput( m_gVideoChannel, m_videoDeviceInputID);
        // result = SGSetChannelPlayFlags  ( m_gVideoChannel, channelPlayFast | channelPlayHighQuality | channelPlayAllData);
        result = SGSetChannelPlayFlags  ( m_gVideoChannel, channelPlayFast );

        VideoDigitizerComponent vdig = SGGetVideoDigitizerComponent(m_gVideoChannel);
        VideoDigitizerError vid_err;
        vid_err = VDSetInputStandard (vdig, palIn);
        OSG_DEBUG << "Setup vdig from input prefs:" << std::endl;
        print_video_component_capability(vdig);

        result = SGVideoDigitizerChanged( m_gVideoChannel);
        result = SGGetSrcVideoBounds    ( m_gVideoChannel, &gActiveVideoRect);
        result = SGSetChannelBounds     ( m_gVideoChannel, &gActiveVideoRect);

        result = SGChangedSource (m_gSeqGrabber, m_gVideoChannel);

        Fixed frame_rate;
        result = SGGetFrameRate (m_gVideoChannel, &frame_rate);
        result = SGSetFrameRate (m_gVideoChannel, 100);
        //
        // Sound
        /*
        long                sound_id;
        Str255              sound_driver_name;
        char*               sound_driver_name_cstr;
        vid_err = VDGetSoundInputSource(vdig, (long)m_videoDeviceInputID, &sound_id);
        vid_err = VDGetSoundInputDriver(vdig, sound_driver_name);
        sound_driver_name_cstr = pstr_printable(sound_driver_name);
        OSG_DEBUG << "vdig sound driver name :" << sound_driver_name_cstr << std::endl;
        OSG_DEBUG << "vdig sound driver id   :" << sound_id << std::endl;
        */
    }
    else
    {
        OSG_FATAL << "Could not create SGNewChannel for Video Channel" << std::endl;
    }
    // Set GWorld back
    SetGWorld(origPort, origDevice);
}


static OSErr MySGDataProc (SGChannel c,Ptr p,long len,long *offset,long chRefCon,TimeValue time,short writeType,long refCon )
{
    QuicktimeLiveImageStream* p_is = (QuicktimeLiveImageStream*)refCon;
    return p_is->dataProcCallback(c,p,len,offset,chRefCon,time,writeType,refCon);
}

OSErr QuicktimeLiveImageStream::dataProcCallback( SGChannel c,Ptr p,long len,long *offset,long chRefCon,TimeValue time,short writeType,long refCon )
{
    OSErr err = noErr;
    //
    OSG_INFO << " Video " << refCon << std::endl;
    dirty();
    //
    return err;
}

// Create the Sequence Grabber DataProc setup for Record
void QuicktimeLiveImageStream::createSequenceGrabberDataProc()
{
    OSErr err;
    err = SGSetDataRef(m_gSeqGrabber, 0, 0, seqGrabToMemory | seqGrabDontMakeMovie);
    if (err != noErr)
    {
       OSG_FATAL << "SGSetDataRef : error" << std::endl;
    }

    // specify a sequence grabber data function
    err = SGSetDataProc(m_gSeqGrabber, NewSGDataUPP(MySGDataProc), (long)this);
    if (err != noErr)
    {
       OSG_FATAL << "SGSetDataProc : error" << std::endl;
    }
}


// Create the Sequence Grabber Audio Channel
void QuicktimeLiveImageStream::createSequenceGrabberAudioChannel()
{
  // Check capability and setting of Sequence Grabber
    GDHandle origDevice;
    CGrafPtr origPort;
    // Create GWorld
    GetGWorld (&origPort, &origDevice);
    SetGWorld (m_gw, NULL); // set current graphics port to offscreen
    // Setup
    // Get a video channel
    ComponentResult result = SGNewChannel (m_gSeqGrabber, SoundMediaType, &m_gSoundChannel);
    if ((m_gSoundChannel != nil) && (result == noErr))
    {
        result = SGInitChannel(m_gSoundChannel, m_gSeqGrabber);
        // result = SGSetChannelUsage (m_gSoundChannel, seqGrabPreview );
        // Usage
        if (g_s_use_sg_record)
            result = SGSetChannelUsage (m_gSoundChannel, seqGrabRecord | seqGrabLowLatencyCapture);
        else
        {
            result = SGSetChannelUsage (m_gSoundChannel, seqGrabPreview | seqGrabRecord | seqGrabLowLatencyCapture);
        }

        // Get
        Str255 deviceName;
        Str255 inputName;
        short  inputNumber;
        result = SGGetChannelDeviceAndInputNames( m_gSoundChannel, deviceName, inputName, &inputNumber);

        // Set
        // OSG_DEBUG << "Setting up audio component from input prefs" << std::endl;
        result = SGSetChannelDevice     ( m_gSoundChannel, m_soundDeviceIDStr);
        result = SGSetChannelDeviceInput( m_gSoundChannel, m_soundDeviceInputID);
        // Set the volume low to prevent feedback when we start the preview,
        // in case the mic is anywhere near the speaker.
        short volume = 0;
        result = SGGetChannelVolume (m_gSoundChannel, &volume );
        // result = SGSetChannelVolume (m_gSoundChannel, 255);
        // Inform
        result = SGChangedSource        ( m_gSeqGrabber,   m_gSoundChannel);
    }
    else
    {
        OSG_FATAL << "Could not create SGNewChannel for Sound Channel" << std::endl;
    }
    // Set GWorld back
    SetGWorld(origPort, origDevice);
}

// GrabFrameCompleteProc (QT callback)
static ComponentResult GrabFrameCompleteProc(SGChannel sgChan, short nBufferNum, Boolean *pbDone, long lRefCon)
{
    QuicktimeLiveImageStream* p_is = (QuicktimeLiveImageStream*)lRefCon;
    return p_is->grabFrameCompleteProc(sgChan, nBufferNum, pbDone, lRefCon);
}

// GrabFrameCompleteProc (QuicktimeLiveImageStream)
ComponentResult QuicktimeLiveImageStream::grabFrameCompleteProc(SGChannel sgChan, short nBufferNum, Boolean *pbDone, long lRefCon)
{
   ComponentResult err = noErr;

   // call the default grab-complete function
   err = SGGrabFrameComplete(sgChan,      // channel reference
                             nBufferNum,  // buffer identifier, provided for you
                             pbDone);     // pointer to a boolean, has the frame been completely captured? provided for you

   static unsigned int fps_counter = 0;
   static osg::Timer_t start, finish;

   if (fps_counter == 0)
       start = osg::Timer::instance()->tick();
   // if the frame is done, make sure the Image is replaced
   if (*pbDone && (sgChan == m_gVideoChannel))
   {
        dirty();
        ++fps_counter;
        if (fps_counter == 100)
        {
            finish = osg::Timer::instance()->tick();
            double dur = osg::Timer::instance()->delta_s(start, finish);
            double fps = 100.0 / dur;
            OSG_NOTICE << "Executed 100 frames in " << dur << " seconds : ~" << fps << " fps" << std::endl;
            fps_counter = 0;
        }
   }

   return err;
}


// Create callbacks
void QuicktimeLiveImageStream::createSequenceGrabberVideoBottlenecks()
{
    OSErr  err = noErr;
    // set the value of a reference constant that is passed to the callback functions
    err = SGSetChannelRefCon(m_gVideoChannel, (long)this);
    if (err == noErr)
    {
        VideoBottles  vb;
        // get the current bottlenecks
        vb.procCount = 9;
        err = SGGetVideoBottlenecks(m_gVideoChannel, &vb);
        if (err == noErr)
        {
            // add our GrabFrameComplete function
            vb.grabCompleteProc = NewSGGrabCompleteBottleUPP(GrabFrameCompleteProc);
            err = SGSetVideoBottlenecks(m_gVideoChannel, &vb);
        }
    }
}


// 2.
// CreateAndRunWithVideoDigitizer
void QuicktimeLiveImageStream::createAndRunWithVideoDigitizer(std::string fileName)
{
   std::string::size_type idx = fileName.find(':');
   if (idx == std::string::npos)
   {
       OSG_FATAL << "Error while parsing deviceID:deviceInputID.live path : " << fileName << std::endl;
   }
   // Better c++ code is to use istrstream
   std::string deviceIDStr      = fileName.substr(0,idx);
   std::string deviceInputIDStr = fileName.substr(idx+1);
   m_videoDeviceID      = static_cast<short>(atoi(deviceIDStr.c_str()));
   m_videoDeviceInputID = static_cast<short>(atoi(deviceInputIDStr.c_str()));
   // Get Video Digitizer Rectangle bounds from a Sequence Grabber proxy (using IDs)
   get_video_device_bounds_idstr(m_videoDeviceID, m_videoDeviceInputID, m_videoRectWidth, m_videoRectHeight, m_videoDeviceIDStr);
   // Create the Image
   createImage();
   // Create the offscreen GWorld (using Image  as target memory)
   createGWorld();
   // Create the Sequence Grabber (using GWorld as target memory)
   createVideoDigitizer();
   // Go
   _status = ImageStream::PLAYING;

   VideoDigitizerError error = VDSetPlayThruOnOff(m_vdig, vdPlayThruOn);
   if (error != noErr)
   {
       OSG_FATAL << "VDSetPlayThruOnOff : error" << std::endl;
   }
   // Ticker
   start();
}

// 2.
// Create the Video Digitizer (using GWorld Pixmap as target memory)
void QuicktimeLiveImageStream::createVideoDigitizer()
{
    // #define videoDigitizerComponentType = 'vdig'
    ComponentDescription video_component_description;
    video_component_description.componentType         = 'vdig'; /* A unique 4-byte code identifying the command set */
    video_component_description.componentSubType      = 0;      /* Particular flavor of this instance */
    video_component_description.componentManufacturer = 0;      /* Vendor identification */
    video_component_description.componentFlags        = 0;      /* 8 each for Component,Type,SubType,Manuf/revision */
    video_component_description.componentFlagsMask    = 0;      /* Mask for specifying which flags to consider in search, zero during registration */
    long num_video_components = CountComponents (&video_component_description);
    OSG_DEBUG << " available Video DigitizerComponents : " << num_video_components << std::endl;
    if (num_video_components)
    {
        Component aComponent = 0;
        short     aDeviceID  = 0;
        do
        {
            ComponentDescription full_video_component_description = video_component_description;
            aComponent = FindNextComponent(aComponent, &full_video_component_description);
            if (aComponent && (aDeviceID == m_videoDeviceID))
            {
                OSG_DEBUG << "Component" << std::endl;
                OSErr                err;
                Handle compName = NewHandle(256);
                Handle compInfo = NewHandle(256);
                err = GetComponentInfo( aComponent, &full_video_component_description, compName,compInfo,0);
                OSG_DEBUG << "    Name: " << pstr_printable((StringPtr)*compName) << std::endl;
                OSG_DEBUG << "    Desc: " << pstr_printable((StringPtr)*compInfo) << std::endl;
                //Capabilities
                VideoDigitizerComponent component_instance = OpenComponent(aComponent);
                m_vdig = component_instance;
                //Setup
                // Onscreen
                // Check capability and setting of Sequence Grabber
                GDHandle origDevice;
                CGrafPtr origPort;
                GetGWorld (&origPort, &origDevice);
                VideoDigitizerError error;
                Rect                destinationBounds;
                destinationBounds.left   = 0;
                destinationBounds.top    = 0;
                destinationBounds.right  = m_videoRectWidth;
                destinationBounds.bottom = m_videoRectHeight;
                error = VDSetPlayThruDestination(m_vdig, m_pixmap, &destinationBounds, 0, 0);
                //error = VDSetPlayThruGlobalRect(m_vdig, (GrafPtr)origPort, &destinationBounds);
                if (error != noErr)
                {
                    OSG_FATAL << "VDSetPlayThruDestination : error" << std::endl;
                }
                print_video_component_capability(component_instance);
                break;
            }
            ++aDeviceID;
        }
        while (0 != aComponent);
     }
}


// Thread run method
void QuicktimeLiveImageStream::run()
{
   ComponentResult result = noErr;
   bool            done   = false;

   //memset( data(), 255, 720*250*4);

   while (!done)
   {
       // Do some funky rotational memset
       // void * memset ( void * ptr, int value, size_t num );
       //memset
       // dirty();
       if (g_s_use_sg)
       {
           result = SGIdle(m_gSeqGrabber);
           if (result != noErr)
           {
               OSG_FATAL << "SGIdle : error" << std::endl;
           }
       }
       //OpenThreads::Thread::microSleep(250000); // 25fps (1,000,000 = 1 fps)
       //OpenThreads::Thread::microSleep(50000); // 200fps (1,000,000 = 1 fps)
       //OpenThreads::Thread::microSleep(25000); // 400fps (1,000,000 = 1 fps)
       // Ridiculous
       OpenThreads::Thread::microSleep(10000); // 1000fps (1,000,000 = 1 fps)
   }
}




