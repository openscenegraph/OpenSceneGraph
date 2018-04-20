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

#ifndef _QUICKTIMELIVEIMAGESTREAM_H_
#define _QUICKTIMELIVEIMAGESTREAM_H_

#include <osg/ImageStream>

#include <OpenThreads/Thread>
#include <OpenThreads/Mutex>

#ifdef __APPLE__
   #include <QuickTime/QuickTime.h>
   #include <Carbon/Carbon.h>
   #define QT_HANDLE_IMAGES_ALSO
#else
   #include <QTML.h>
   #include <Movies.h>
   #include <Quickdraw.h>
   #include <QDOffscreen.h>
   #include <QuicktimeComponents.h>
   #include <FixMath.h>
   #include <CGBitmapContext.h>
   #include <CGImage.h>
   #include <CGColorSpace.h>
   #include <ImageCompression.h>
   #include <TextUtils.h>
#endif


/**
* Quicktime Live/Video Image Stream class.
* Streams a quicktime live video feed into an image
*/
class QuicktimeLiveImageStream : public osg::ImageStream, public OpenThreads::Thread
{
public:
  /// Constructor
  QuicktimeLiveImageStream(std::string fileName = "");
  /// destructor
  virtual ~QuicktimeLiveImageStream();

  virtual Object* clone() const { return new QuicktimeLiveImageStream; }
  virtual bool isSameKindAs(const Object* obj) const
  {
     return dynamic_cast<const QuicktimeLiveImageStream*>(obj) != NULL;
  }
  virtual const char* className() const { return "QuicktimeLiveImageStream"; }

  /// Start or continue stream.
  virtual void play();
  /// Pause stream at current position.
  virtual void pause();
  /// stop playing
  virtual void quit(bool waitForThreadToExit);

public:
   /// Do more than load - it's live!
   void load(std::string fileName);
   // Create the Image
   void createImage();
   // Create the offscreen GWorld (using Image  as target memory)
   void createGWorld();

   // 1.
   // CreateAndRunWithSequenceGrabber
       void createAndRunWithSequenceGrabber(std::string fileName);
   // Create the Sequence Grabber (using GWorld as target memory)
   void createSequenceGrabber();
   // Create the Sequence Grabber Video Channel
   void createSequenceGrabberVideoChannel();
   // Create the Sequence Grabber DataProc setup for Record
   void  createSequenceGrabberDataProc();
   OSErr dataProcCallback( SGChannel c,Ptr p,long len,long *offset,long chRefCon,TimeValue time,short writeType,long refCon );
   // Create the Sequence Grabber Audio Channel
   void createSequenceGrabberAudioChannel();
       // Create callbacks
       ComponentResult grabFrameCompleteProc(SGChannel sgChan, short nBufferNum, Boolean *pbDone, long lRefCon);
       void createSequenceGrabberVideoBottlenecks();

   // 2.
   // CreateAndRunWithVideoDigitizer
       void createAndRunWithVideoDigitizer(std::string fileName);
   // Create the Video Digitizer (using GWorld Pixmap as target memory)
   void createVideoDigitizer();

   // Thread run method
   virtual void run();
   //
   short m_videoDeviceID, m_videoDeviceInputID;
   Str63 m_videoDeviceIDStr;
   short m_videoRectWidth, m_videoRectHeight;
       //
       short m_soundDeviceID, m_soundDeviceInputID;
   Str63 m_soundDeviceIDStr;
   // QuickTime stuff
   GWorldPtr               m_gw;
   // SG
   SeqGrabComponent        m_gSeqGrabber;
   SGChannel                   m_gVideoChannel;
   SGChannel                   m_gSoundChannel;
   // VD
   VideoDigitizerComponent m_vdig;
   PixMapHandle            m_pixmap;
};


#endif
