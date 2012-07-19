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


#include <cstdio>
#include <cstdlib>
#include <string>
#include <sstream>

#include "osg/Image"
#include "osg/Notify"
#include "osg/Geode"
#include "osg/GL"

#include "osgDB/FileNameUtils"
#include "osgDB/Registry"
#include "osgDB/FileUtils"

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

#include "QTLiveUtils.h"


// Utils
char* pstr_printable(StringPtr src_pstr)
{
    char* dst_cstr = new char[256];
    p2cstrcpy(dst_cstr, src_pstr);
    return dst_cstr;
}


void initialize_quicktime_qtml()
{
    OSG_NOTICE << "QT QTML: Starting up... " << std::endl;
    OSErr err;
#ifndef __APPLE__
    err = InitializeQTML(0);
    if (err!=0)
    {
        OSG_FATAL << "Error while initializing quicktime QTML: " << err << std::endl;
    }
    else
    {
        OSG_NOTICE << "QT QTML: initialized successfully"  << std::endl;
    }
#endif
}

void terminite_quicktime_qtml()
{
    OSG_NOTICE << "QT QTML: Closing down... " << std::endl;
#ifndef __APPLE__
    TerminateQTML();
#endif
    OSG_NOTICE << "QT QTML: Closed successfully"  << std::endl;
}

void enter_quicktime_movies()
{
    OSG_NOTICE << "QT Movies: Starting up... " << std::endl;
    OSErr err;
    err = EnterMovies();
    if (err!=0)
    {
        OSG_FATAL << "Error while initializing Movies: " << err << std::endl;
    }
    else
    {
        OSG_NOTICE << "QT Movies: initialized successfully"  << std::endl;
    }
}

void leave_quicktime_movies()
{
    OSG_NOTICE << "QT Movies: Closing down... " << std::endl;
#ifndef __APPLE__
    ExitMovies();
#endif
    OSG_NOTICE << "QT Movies: closed successfully"  << std::endl;
}

#if TARGET_OS_MAC
void enter_quicktime_movies_mt()
{
    OSG_NOTICE << "QT Movies MT: Starting up... " << std::endl;
    OSErr err;
    err = EnterMoviesOnThread(0);
    if (err!=0)
    {
        OSG_FATAL << "Error while initializing Movies MT: " << err << std::endl;
    }
    else
    {
        OSG_NOTICE << "QT Movies MT: initialized successfully"  << std::endl;
    }
}

void leave_quicktime_movies_mt()
{
    OSG_NOTICE << "QT Movies MT: Closing down... " << std::endl;
#ifndef __APPLE__
    ExitMoviesOnThread();
#endif
    OSG_NOTICE << "QT Movies MT: closed successfully"  << std::endl;
}
#endif



QTScopedQTMLInitialiser::QTScopedQTMLInitialiser()
{
    initialize_quicktime_qtml();
}
QTScopedQTMLInitialiser::~QTScopedQTMLInitialiser()
{
    terminite_quicktime_qtml();
}


QTScopedMovieInitialiser::QTScopedMovieInitialiser()
{
    enter_quicktime_movies();
}
QTScopedMovieInitialiser::~QTScopedMovieInitialiser()
{
    leave_quicktime_movies();
}


#if TARGET_OS_MAC
QTScopedMovieInitialiser_MT::QTScopedMovieInitialiser_MT()
{
    enter_quicktime_movies_mt();
}
QTScopedMovieInitialiser_MT::~QTScopedMovieInitialiser_MT()
{
    leave_quicktime_movies_mt();
}
#endif


// DigitizerInfo input/output Capability checker
bool supports_capability( long input_flags, long option_flags )
{
    long result_l = (input_flags & option_flags);
    return result_l == option_flags;
}

// Capability
void print_video_component_capability(VideoDigitizerComponent aComponent)
{
    // Returns capability and status information about a specified video digitizer component.
    VideoDigitizerError vid_err;
    DigitizerInfo       vid_info;
    // Capability flags
    OSG_NOTICE << std::endl;
    vid_err = VDGetDigitizerInfo(aComponent, &vid_info);
    if (vid_err)
    {
        OSG_NOTICE << "VDGetDigitizerInfo(aComponent, &vid_info) - ERROR" << std::endl;
    }
    else
    {
        OSG_NOTICE << "DigitizerInfo:" << std::endl;
        short vdigType = vid_info.vdigType;
        if (vdigType == vdTypeBasic) { OSG_NOTICE << "Digitizer Type : Basic (no clipping)" << std::endl; }
        if (vdigType == vdTypeAlpha) { OSG_NOTICE << "Digitizer Type : Alpha clipping" << std::endl; }
        if (vdigType == vdTypeMask)  { OSG_NOTICE << "Digitizer Type : Mask Plane clipping" << std::endl; }
        if (vdigType == vdTypeKey)   { OSG_NOTICE << "Digitizer Type : Key Color(s) clipping" << std::endl; }
        short vdigSlot = vid_info.slot;
        OSG_NOTICE << "Hardwre Slot : " << vdigSlot << std::endl;
        OSG_NOTICE << "Input Capability:" << std::endl << std::boolalpha;
        long inputCapabilityFlags = vid_info.inputCapabilityFlags;
        OSG_NOTICE << "    NTSC      : " << supports_capability(inputCapabilityFlags, digiInDoesNTSC) << std::endl;
        OSG_NOTICE << "    PAL       : " << supports_capability(inputCapabilityFlags, digiInDoesPAL)  << std::endl;
        OSG_NOTICE << "    Composite : " << supports_capability(inputCapabilityFlags, digiInDoesComposite) << std::endl;
        OSG_NOTICE << "    Component : " << supports_capability(inputCapabilityFlags, digiInDoesComponent) << std::endl;
        OSG_NOTICE << "    SVideo    : " << supports_capability(inputCapabilityFlags, digiInDoesSVideo) << std::endl;
        OSG_NOTICE << "Input Current:" << std::endl;
        long inputCurrentFlags = vid_info.inputCurrentFlags;
        OSG_NOTICE << "    NTSC      : " << supports_capability(inputCurrentFlags, digiInDoesNTSC) << std::endl;
        OSG_NOTICE << "    PAL       : " << supports_capability(inputCurrentFlags, digiInDoesPAL)  << std::endl;
        OSG_NOTICE << "    Composite : " << supports_capability(inputCurrentFlags, digiInDoesComposite) << std::endl;
        OSG_NOTICE << "    Component : " << supports_capability(inputCurrentFlags, digiInDoesComponent) << std::endl;
        OSG_NOTICE << "    SVideo    : " << supports_capability(inputCurrentFlags, digiInDoesSVideo) << std::endl;
        // Heights
        short minDestHeight = vid_info.minDestHeight;
        short minDestWidth  = vid_info.minDestWidth;
        short maxDestWidth  = vid_info.maxDestWidth;
        short maxDestHeight = vid_info.maxDestHeight;
        OSG_NOTICE << "Min destination width,height :  " << minDestWidth << "  " << minDestHeight << std::endl;
        OSG_NOTICE << "Max destination width,height :  " << maxDestWidth << "  " << maxDestHeight << std::endl;
        // Current Status
        long inputFlags, outputFlags;
        vid_err = VDGetCurrentFlags(aComponent, &inputFlags, &outputFlags);
        OSG_NOTICE << "    NTSC          : " << supports_capability(inputFlags, digiInDoesNTSC) << std::endl;
        OSG_NOTICE << "    PAL           : " << supports_capability(inputFlags, digiInDoesPAL)  << std::endl;
        OSG_NOTICE << "    Composite     : " << supports_capability(inputFlags, digiInDoesComposite) << std::endl;
        OSG_NOTICE << "    Component     : " << supports_capability(inputFlags, digiInDoesComponent) << std::endl;
        OSG_NOTICE << "    SVideo        : " << supports_capability(inputFlags, digiInDoesSVideo) << std::endl;
        OSG_NOTICE << "    GenLock       : " << supports_capability(inputFlags, digiInDoesGenLock) << std::endl;
        OSG_NOTICE << "    SECAM         : " << supports_capability(inputFlags, digiInDoesSECAM) << std::endl;
        OSG_NOTICE << "    VTR_Broadcast : " << supports_capability(inputFlags, digiInVTR_Broadcast) << std::endl;
        OSG_NOTICE << "    Color         : " << supports_capability(inputFlags, digiInDoesColor) << std::endl;
        OSG_NOTICE << "    BW            : " << supports_capability(inputFlags, digiInDoesBW) << std::endl;
        OSG_NOTICE << "   *SignalLock*   : " << supports_capability(inputFlags, digiInSignalLock) << std::endl;
        // Preferrd Width Height
        long pref_width, pref_height;
        vid_err = VDGetPreferredImageDimensions(aComponent, &pref_width, &pref_height);
        if (vid_err) { OSG_NOTICE << "VDGetPreferredImageDimensions(aComponent, &pref_width, &pref_height) - ERROR" << std::endl; }
        else         { OSG_NOTICE << "Preferrred width,height :  " << pref_width << "  " << pref_height << std::endl; }

        // Inputs
        short inputs;
        vid_err = VDGetNumberOfInputs(aComponent, &inputs);
        if (vid_err) { OSG_NOTICE << "VDGetNumberOfInputs(aComponent, &inputs) - ERROR" << std::endl; }
        else         { OSG_NOTICE << "Number of inputs        :  " << inputs << std::endl; }

        for (short i=0; i <= inputs; ++i)
        {
            Str255 name;
            vid_err = VDGetInputName(aComponent,(long)i, name);
            if (vid_err) { OSG_NOTICE << "VDGetInputName(aComponent,(long)i, name) - ERROR" << std::endl; }
            else         { OSG_NOTICE << "Name of input   " << i << " :  " << pstr_printable(name) << std::endl; }
            short input_format;
            vid_err = VDGetInputFormat(aComponent,(long)i, &input_format);
            if (vid_err) { OSG_NOTICE << "VDGetInputFormat(aComponent,(long)i, &input_format) - ERROR" << std::endl; }
            else
            {
                if (input_format == compositeIn)        { OSG_NOTICE << "Format of input :  compositeIn" << std::endl; }
                if (input_format == sVideoIn)           { OSG_NOTICE << "Format of input :  sVideoIn" << std::endl; }
                if (input_format == rgbComponentIn)     { OSG_NOTICE << "Format of input :  rgbComponentIn" << std::endl; }
                if (input_format == rgbComponentSyncIn) { OSG_NOTICE << "Format of input :  rgbComponentSyncIn" << std::endl; }
                if (input_format == yuvComponentIn)     { OSG_NOTICE << "Format of input :  yuvComponentIn" << std::endl; }
                if (input_format == yuvComponentSyncIn) { OSG_NOTICE << "Format of input :  yuvComponentSyncIn" << std::endl; }
                if (input_format == sdiIn)              { OSG_NOTICE << "Format of input :  sdiIn" << std::endl; }
            }
        }
        // CURRENT Input
        short active_input;
        vid_err = VDGetInput(aComponent, &active_input);
        if (vid_err) { OSG_NOTICE << "VDGetInput(aComponent, &active_input) - ERROR" << std::endl; }
        else         { OSG_NOTICE << "Currently active input :  " << active_input << std::endl; }
    }
}

void probe_video_digitizer_components()
{
      // Extra scopes for DEBUG and breakpoint/stack checking plus QT init/destroy
      {
          // Begin QuickTime
          QTScopedQTMLInitialiser  qt_init;
          QTScopedMovieInitialiser qt_movie_init;

          // #define videoDigitizerComponentType = 'vdig'
          ComponentDescription video_component_description;
          video_component_description.componentType         = 'vdig'; /* A unique 4-byte code indentifying the command set */
          video_component_description.componentSubType      = 0;      /* Particular flavor of this instance */
          video_component_description.componentManufacturer = 0;      /* Vendor indentification */
          video_component_description.componentFlags        = 0;      /* 8 each for Component,Type,SubType,Manuf/revision */
          video_component_description.componentFlagsMask    = 0;      /* Mask for specifying which flags to consider in search, zero during registration */
          long num_video_components = CountComponents (&video_component_description);
          OSG_NOTICE << " available Video DigitizerComponents : " << num_video_components << std::endl;
          if (num_video_components)
          {
              Component aComponent = 0;
              do
              {
                  ComponentDescription full_video_component_description = video_component_description;
                  aComponent = FindNextComponent(aComponent, &full_video_component_description);
                  if (aComponent)
                  {
                      OSG_NOTICE << "Component" << std::endl;
                      OSErr                err;
                      Handle compName = NewHandle(256);
                      Handle compInfo = NewHandle(256);
                      err = GetComponentInfo( aComponent, &full_video_component_description, compName,compInfo,0);
                      OSG_NOTICE << "    Name: " << pstr_printable((StringPtr)*compName) << std::endl;
                      OSG_NOTICE << "    Desc: " << pstr_printable((StringPtr)*compInfo) << std::endl;
                      //Capabilities
                      VideoDigitizerComponent component_instance = OpenComponent(aComponent);
                      print_video_component_capability(component_instance);
                      CloseComponent(component_instance);
                  }
              }
              while (0 != aComponent);
          }
          // End QuickTime
      }
}

OSG_SGDeviceList print_sequence_grabber_device_list(SGDeviceList deviceList)
{
    short count         = (*deviceList)->count;
    short selectedIndex = (*deviceList)->selectedIndex;
    OSG_NOTICE << "DeviceList : " << count << " devices in total" << std::endl;
    OSG_NOTICE << "DeviceList : " << selectedIndex << " is current device" << std::endl;

    // Create List
    OSG_SGDeviceList device_list;
    OSG_SGDevicePair device_pair;
    for (short i=0; i<count; ++i)
    {
        // Devices
        OSG_NOTICE << std::endl;
        SGDeviceName deviceNameRec = (*deviceList)->entry[i];
        Str63        deviceNameStr;
        memcpy(deviceNameStr, deviceNameRec.name, sizeof(Str63));
        OSG_NOTICE << "    " << "Device ID : " << i << "  : DeviceNameStr : " << pstr_printable(deviceNameStr) << std::endl;
        SGDeviceInputList deviceInputList = deviceNameRec.inputs;
        if (deviceInputList)
        {
            // Inputs
            short inputCount         = (*deviceInputList)->count;
            short inputSelectedIndex = (*deviceInputList)->selectedIndex;
            OSG_NOTICE << "    " << "InputList : " << inputCount << " inputs in total" << std::endl;
            OSG_NOTICE << "    " << "InputList : " << inputSelectedIndex << " is current input" << std::endl;
            for (short inp=0; inp<inputCount; ++inp)
            {
                SGDeviceInputName inputNameRec = (*deviceInputList)->entry[inp];
                Str63             inputNameStr;
                memcpy(inputNameStr, inputNameRec.name, sizeof(Str63));
                OSG_NOTICE << "        " << "InputNameStr : " << inp << " " << pstr_printable(inputNameStr) << std::endl;
                // Build up device list
                std::ostringstream os;
                os << i << ":" << inp << ".live";
                device_pair.first  = os.str();
                device_pair.second = std::string(pstr_printable(deviceNameStr)) + std::string("    ") + std::string(pstr_printable(inputNameStr));
                // Append
                device_list.push_back(device_pair);
            }
        }
        else
        {
            OSG_NOTICE << "    InputList is empty!" << std::endl;
        }
    }
    return device_list;
}

std::vector<OSG_SGDeviceList> probe_sequence_grabber_components()
{
      // Create List
      std::vector<OSG_SGDeviceList> devices_list;
      OSG_SGDeviceList              device_list;
      // Extra scopes for DEBUG and breakpoint/stack checking plus QT init/destroy
      {
          // Begin QuickTime
          QTScopedQTMLInitialiser  qt_init;
          QTScopedMovieInitialiser qt_movie_init;

          // #define videoDigitizerComponentType = 'vdig'
          ComponentDescription sg_component_description;
          sg_component_description.componentType         = SeqGrabComponentType; /* A unique 4-byte code indentifying the command set */
          sg_component_description.componentSubType      = 0L;      /* Particular flavor of this instance */
          sg_component_description.componentManufacturer = 'appl';  /* Vendor indentification */
          sg_component_description.componentFlags        = 0L;      /* 8 each for Component,Type,SubType,Manuf/revision */
          sg_component_description.componentFlagsMask    = 0L;      /* Mask for specifying which flags to consider in search, zero during registration */
          long num_sg_components = CountComponents (&sg_component_description);
          OSG_NOTICE << " available SequenceGrabber Components : " << num_sg_components << std::endl;
          if (num_sg_components)
          {
              Component aComponent = 0;
              do
              {
                  ComponentDescription full_sg_component_description = sg_component_description;
                  aComponent = FindNextComponent(aComponent, &full_sg_component_description);
                  if (aComponent)
                  {
                      OSG_NOTICE << "Component" << std::endl;
                      OSErr                err;
                      Handle compName = NewHandle(256);
                      Handle compInfo = NewHandle(256);
                      err = GetComponentInfo( aComponent, &full_sg_component_description, compName,compInfo,0);
                      OSG_NOTICE << "    Name: " << pstr_printable((StringPtr)*compName) << std::endl;
                      OSG_NOTICE << "    Desc: " << pstr_printable((StringPtr)*compInfo) << std::endl;
                      SeqGrabComponent gSeqGrabber;
                      SGChannel           gVideoChannel;
                      SGChannel           gSoundChannel;
                      Rect               gActiveVideoRect;
                      gSeqGrabber = OpenComponent (aComponent);
                      // If we got a sequence grabber, set it up
                      if (gSeqGrabber != 0L)
                      {
                          ComponentResult result = noErr;
                          // Initialize the sequence grabber
                          result = SGInitialize (gSeqGrabber);
                          if (result == noErr)
                          {
                              // Check capability and setting of Sequence Grabber
                              Rect         destinationBounds;
                              OSStatus     err;
                              GDHandle     origDevice;
                              CGrafPtr     origPort;
                              GWorldPtr    gw;
                              PixMapHandle pixmap = NULL;
                              int*         destinationData = new int [1024*1024]; // 1024*1024*4 bytes (32bit RGBA)
                              destinationBounds.left   = 0;
                              destinationBounds.top    = 0;
                              destinationBounds.right  = 2048;
                              destinationBounds.bottom = 2048;
                              err = QTNewGWorldFromPtr(&gw, k32ARGBPixelFormat, &destinationBounds,
                                                       NULL, NULL, 0, (Ptr)destinationData, 4*1024);
                              if (err !=0 )
                              {
                                  OSG_FATAL << "Could not create gWorld" << std::endl;
                              }
                              else
                              {
                                  // Create GWorld
                                  GetGWorld (&origPort, &origDevice);
                                  SetGWorld (gw, NULL); // set current graphics port to offscreen
                                  pixmap = GetGWorldPixMap (gw);
                                  if (pixmap)
                                  {
                                      if (!LockPixels (pixmap)) // lock offscreen pixel map
                                      {
                                          OSG_FATAL << "Could not lock PixMap" << std::endl;
                                      }
                                  }
                                  // Set GWorld
                                  result = SGSetGWorld(gSeqGrabber, (CGrafPtr)gw, 0);
                                  // Set GWorld back
                                  // SetGWorld(origPort, origDevice);
                                  if (result != noErr)
                                  {
                                      OSG_FATAL << "Could not set GWorld on SG" << std::endl;
                                  }
                                  else
                                  {
                                      // Get a video channel
                                      result = SGNewChannel (gSeqGrabber, VideoMediaType, &gVideoChannel);
                                      if ((gVideoChannel != nil) && (result == noErr))
                                      {
                                          // Init
                                          // result = SGInitChannel(gVideoChannel, gSeqGrabber);
//                                           if (result != noErr)
//                                           {
//                                               OSG_NOTICE << "SGInitChannel - failed!" << std::endl;
//                                           }
                                          // Usage
                                          result = SGSetChannelUsage (gVideoChannel, seqGrabPreview);
                                          // Bounds
                                          result = SGGetSrcVideoBounds (gVideoChannel, &gActiveVideoRect);
                                          OSG_NOTICE << "SrcVideoBounds: " << gActiveVideoRect.right << " " << gActiveVideoRect.bottom << std::endl;
                                          Str255 deviceName;
                                          Str255 inputName;
                                          short  inputNumber;
                                          result = SGGetChannelDeviceAndInputNames( gVideoChannel, deviceName, inputName, &inputNumber);
                                          if (result != noErr)
                                          {
                                              OSG_NOTICE << "Could not get DeviceAndInput names from Video SG" << std::endl;
                                          }
                                          OSG_NOTICE << "ChannelDeviceAndInputNamesNumber: " << pstr_printable(deviceName) << " : " << pstr_printable(inputName) << " : " << inputNumber << std::endl;
                                          SGDeviceList deviceList;
                                          result = SGGetChannelDeviceList( gVideoChannel, sgDeviceListIncludeInputs, &deviceList);
                                          if (result != noErr)
                                          {
                                              OSG_NOTICE << "Could not get DeviceList from Video SG" << std::endl;
                                          }
                                          else
                                          {
                                              OSG_NOTICE << "DeviceList from Video SG ok" << std::endl;
                                              device_list = print_sequence_grabber_device_list(deviceList);
                                              devices_list.push_back(device_list);
                                          }
                                      }
                                      // Get a sound channel
                                      result = SGNewChannel (gSeqGrabber, SoundMediaType, &gSoundChannel);
                                      if ((gSoundChannel != nil) && (result == noErr))
                                      {
                                          // Usage
                                          result = SGSetChannelUsage (gSoundChannel, seqGrabPreview);
                                          Str255 deviceName;
                                          Str255 inputName;
                                          short  inputNumber;
                                          result = SGGetChannelDeviceAndInputNames( gVideoChannel, deviceName, inputName, &inputNumber);
                                          if (result != noErr)
                                          {
                                              OSG_NOTICE << "Could not get DeviceAndInput names from Sound SG" << std::endl;
                                          }
                                          OSG_NOTICE << "ChannelDeviceAndInputNamesNumber: " << pstr_printable(deviceName) << " : " << pstr_printable(inputName) << " : " << inputNumber << std::endl;
                                          SGDeviceList deviceList;
                                          result = SGGetChannelDeviceList( gSoundChannel, sgDeviceListIncludeInputs, &deviceList);
                                          if (result != noErr)
                                          {
                                              OSG_NOTICE << "Could not get DeviceList from Sound SG" << std::endl;
                                          }
                                          else
                                          {
                                              OSG_NOTICE << "DeviceList from Sound SG ok" << std::endl;
                                              device_list = print_sequence_grabber_device_list(deviceList);
                                              devices_list.push_back(device_list);
                                          }
                                      }
                                  }
                              SetGWorld(origPort, origDevice);
                              DisposeGWorld(gw);
                              }
                          }
                      }
                      SGDisposeChannel(gSeqGrabber, gVideoChannel);
                      CloseComponent(gSeqGrabber);
                  }
              }
              while (0 != aComponent);
          }
      // End QuickTime
      }
      return devices_list;
}


void get_video_device_bounds_idstr(short deviceID, short deviceInputID, short& out_width, short& out_height, Str63& out_deviceIDStr)
{
      // Extra scopes for DEBUG and breakpoint/stack checking plus QT init/destroy
      {
          // Begin QuickTime
          QTScopedQTMLInitialiser  qt_init;
          QTScopedMovieInitialiser qt_movie_init;

          ComponentDescription sg_component_description;
          sg_component_description.componentType         = SeqGrabComponentType; /* A unique 4-byte code indentifying the command set */
          sg_component_description.componentSubType      = 0L;      /* Particular flavor of this instance */
          sg_component_description.componentManufacturer = 0L;      /* Vendor indentification */
          sg_component_description.componentFlags        = 0L;      /* 8 each for Component,Type,SubType,Manuf/revision */
          sg_component_description.componentFlagsMask    = 0L;      /* Mask for specifying which flags to consider in search, zero during registration */
          long num_sg_components = CountComponents (&sg_component_description);
          if (num_sg_components)
          {
              Component aComponent = 0;
              do
              {
                  ComponentDescription full_sg_component_description = sg_component_description;
                  aComponent = FindNextComponent(aComponent, &full_sg_component_description);
                  if (aComponent)
                  {
                      SeqGrabComponent gSeqGrabber;
                      SGChannel           gVideoChannel;
                      Rect               gActiveVideoRect;
                      gSeqGrabber = OpenComponent (aComponent);
                      // If we got a sequence grabber, set it up
                      if (gSeqGrabber != 0L)
                      {
                          ComponentResult result = noErr;
                          // Initialize the sequence grabber
                          result = SGInitialize (gSeqGrabber);
                          if (result == noErr)
                          {
                              // Check capability and setting of Sequence Grabber
                              Rect         destinationBounds;
                              OSStatus     err;
                              GDHandle     origDevice;
                              CGrafPtr     origPort;
                              GWorldPtr    gw;
                              PixMapHandle pixmap = NULL;
                              int*         destinationData = new int [1024*1024]; // 1024*1024*4 bytes (32bit RGBA)
                              destinationBounds.left   = 0;
                              destinationBounds.top    = 0;
                              destinationBounds.right  = 256;
                              destinationBounds.bottom = 256;
                              err = QTNewGWorldFromPtr(&gw, k32ARGBPixelFormat, &destinationBounds,
                                                       NULL, NULL, 0, (Ptr)destinationData, 4*256);
                              if (err !=0 )
                                  OSG_NOTICE << "Could not create gWorld" << std::endl;
                              else
                              {
                                  // Create GWorld
                                  GetGWorld (&origPort, &origDevice);
                                  SetGWorld (gw, NULL); // set current graphics port to offscreen
                                  pixmap = GetGWorldPixMap (gw);
                                  if (pixmap)
                                  {
                                      if (!LockPixels (pixmap))
                                      {
                                          // lock offscreen pixel map
                                          OSG_FATAL << "Could not lock PixMap" << std::endl;
                                      }
                                  }
                                  // Set GWorld
                                  result = SGSetGWorld(gSeqGrabber, (CGrafPtr)gw, 0);
                                  // Set GWorld back
                                  // SetGWorld(origPort, origDevice);
                                  if (result != noErr)
                                  {
                                      OSG_FATAL << "Could not set GWorld on SG" << std::endl;
                                  }
                                  else
                                  {
                                      // Get a video channel
                                      result = SGNewChannel (gSeqGrabber, VideoMediaType, &gVideoChannel);
                                      if ((gVideoChannel != nil) && (result == noErr))
                                      {
                                          result = SGSetChannelUsage (gVideoChannel, seqGrabPreview);
                                          Str255 deviceName;
                                          Str255 inputName;
                                          short  inputNumber;
                                          result = SGGetChannelDeviceAndInputNames( gVideoChannel, deviceName, inputName, &inputNumber);
                                          SGDeviceList deviceList;
                                          result = SGGetChannelDeviceList( gVideoChannel, sgDeviceListIncludeInputs, &deviceList);
                                          short count = (*deviceList)->count;
                                          if (deviceID >= count)
                                          {
                                              OSG_FATAL << "DeviceID : " << deviceID << " too large - we only have " << count << " devices" << std::endl;
                                          }
                                          SGDeviceName deviceNameRec = (*deviceList)->entry[deviceID];
                                          SGDeviceInputList deviceInputList = deviceNameRec.inputs;
                                          if (deviceInputList == 0)
                                          {
                                              OSG_FATAL << "DeviceInputList is empty!" << std::endl;
                                          }
                                          else
                                          {
                                              short inputCount = (*deviceInputList)->count;
                                              if (deviceInputID >= inputCount)
                                              {
                                                  OSG_FATAL << "DeviceInputID : " << deviceInputID << " too large - we only have " << inputCount << " inputs for device" << std::endl;
                                              }
                                          }
                                          // Ok
                                          Str63 deviceNameStr;
                                          memcpy(deviceNameStr, deviceNameRec.name, sizeof(Str63));
                                          // Set
                                          result = SGSetChannelDevice     ( gVideoChannel, deviceNameStr);
                                          result = SGSetChannelDeviceInput( gVideoChannel, deviceInputID);

                                          VideoDigitizerComponent vdig = SGGetVideoDigitizerComponent(gVideoChannel);
                                          VideoDigitizerError vid_err;
                                          vid_err = VDSetInputStandard (vdig, palIn);
                                          result = SGVideoDigitizerChanged( gVideoChannel);

                                          result = SGGetSrcVideoBounds    ( gVideoChannel, &gActiveVideoRect);
                                          OSG_NOTICE << "SrcVideoBounds: " << gActiveVideoRect.right << " " << gActiveVideoRect.bottom << std::endl;
                                          // Out
                                          out_width  = gActiveVideoRect.right;
                                          out_height = gActiveVideoRect.bottom;
                                          memcpy(out_deviceIDStr, deviceNameRec.name, sizeof(Str63));
                                      }
                                  }
                              SetGWorld(origPort, origDevice);
                              DisposeGWorld(gw);
                              }
                          }
                      }
                      SGDisposeChannel(gSeqGrabber, gVideoChannel);
                      CloseComponent(gSeqGrabber);
                  }
              }
              while (0 != aComponent);
          }
      // End QuickTime
      }
}

void get_sound_device_idstr(short soundDeviceID, short soundDeviceInputID, Str63& out_soundDeviceIDStr)
{
          // Extra scopes for DEBUG and breakpoint/stack checking plus QT init/destroy
      {
          // Begin QuickTime
          QTScopedQTMLInitialiser  qt_init;
          QTScopedMovieInitialiser qt_movie_init;

          // #define videoDigitizerComponentType = 'vdig'
          ComponentDescription sg_component_description;
          sg_component_description.componentType         = SeqGrabComponentType; /* A unique 4-byte code indentifying the command set */
          sg_component_description.componentSubType      = 0L;      /* Particular flavor of this instance */
          sg_component_description.componentManufacturer = 0L;      /* Vendor indentification */
          sg_component_description.componentFlags        = 0L;      /* 8 each for Component,Type,SubType,Manuf/revision */
          sg_component_description.componentFlagsMask    = 0L;      /* Mask for specifying which flags to consider in search, zero during registration */
          long num_sg_components = CountComponents (&sg_component_description);
          if (num_sg_components)
          {
              Component aComponent = 0;
              do
              {
                  ComponentDescription full_sg_component_description = sg_component_description;
                  aComponent = FindNextComponent(aComponent, &full_sg_component_description);
                  if (aComponent)
                  {
                      SeqGrabComponent gSeqGrabber;
                      SGChannel           gSoundChannel;
                      gSeqGrabber = OpenComponent (aComponent);
                      // If we got a sequence grabber, set it up
                      if (gSeqGrabber != 0L)
                      {
                          ComponentResult result = noErr;
                          // Initialize the sequence grabber
                          result = SGInitialize (gSeqGrabber);
                          if (result == noErr)
                          {
                              // Check capability and setting of Sequence Grabber
                              // Get a sound channel
                              result = SGNewChannel (gSeqGrabber, SoundMediaType, &gSoundChannel);
                              if ((gSoundChannel != nil) && (result == noErr))
                              {
                                  result = SGSetChannelUsage (gSoundChannel, seqGrabPreview);
                                  Str255 deviceName;
                                  Str255 inputName;
                                  short  inputNumber;
                                  result = SGGetChannelDeviceAndInputNames( gSoundChannel, deviceName, inputName, &inputNumber);
                                  SGDeviceList deviceList;
                                  result = SGGetChannelDeviceList( gSoundChannel, sgDeviceListIncludeInputs, &deviceList);
                                  short count = (*deviceList)->count;
                                  if (soundDeviceID >= count)
                                  {
                                      OSG_FATAL << "DeviceID : " << soundDeviceID << " too large - we only have " << count << " devices" << std::endl;
                                  }
                                  SGDeviceName deviceNameRec = (*deviceList)->entry[soundDeviceID];
                                  SGDeviceInputList deviceInputList = deviceNameRec.inputs;
                                  short inputCount = (*deviceInputList)->count;
                                  if (soundDeviceInputID >= inputCount)
                                  {
                                      OSG_FATAL << "DeviceInputID : " << soundDeviceInputID << " too large - we only have " << inputCount << " inputs for device" << std::endl;
                                  }
                                  // Ok
                                  Str63 deviceNameStr;
                                  memcpy(deviceNameStr, deviceNameRec.name, sizeof(Str63));
                                  // Set
                                  result = SGSetChannelDevice     ( gSoundChannel, deviceNameStr);
                                  result = SGSetChannelDeviceInput( gSoundChannel, soundDeviceInputID);
                                  // Out
                                  memcpy(out_soundDeviceIDStr, deviceNameRec.name, sizeof(Str63));

                                  SGDisposeChannel(gSeqGrabber, gSoundChannel);
                              }
                          }
                          CloseComponent(gSeqGrabber);
                      }
                  }
              }
              while (0 != aComponent);
          }
      // End QuickTime
      }
}



// Getting Information About Video Digitizer Components
// You can use the VDGetDigitizerInfo function in your application to retrieve
// information about the capabilities of a video digitizer component. You can use
// the VDGetCurrentFlags function to obtain current status information from a video digitizer component.

// Setting Source Characteristics
// You can use the VDGetMaxSrcRect function in your application to get the size and location of the maximum
// source rectangle. Similarly, the VDGetActiveSrcRect function allows you to get this information about
// the active source rectangle, and the VDGetVBlankRect function enables you to obtain information about the vertical blanking rectangle.
// You can use the VDSetDigitizerRect function to set the size and location of the digitizer rectangle.
// The VDGetDigitizerRect function lets you retrieve the size and location of this rectangle.

// Imput Source
// Some of these functions provide information about the available video inputs. Applications can use
// the VDGetNumberOfInputs function to determine the number of video inputs supported by the digitizer component.
// The VDGetInputFormat function allows applications to find out the video format (composite, s-video, or component) employed by a specified input.
// You can use the VDSetInput function in your application to specify the input to be used by the digitizer component.
// The VDGetInput function returns the currently selected input.
// The VDSetInputStandard function allows you to specify the video signaling standard to be used by the video digitizer component.

/*
QTVideoOutputRestoreState
QTVideoOutputSaveState

 Selecting an Input Source
VDGetInput
VDGetInputFormat
VDGetNumberOfInputs
VDSetInput
VDSetInputStandard
 Setting Source Characteristics
VDGetActiveSrcRect
VDGetDigitizerRect
VDGetMaxSrcRect
VDGetVBlankRect
VDSetDigitizerRect
 Setting Video Destinations
VDGetMaxAuxBuffer
VDGetPlayThruDestination
VDPreflightDestination
VDPreflightGlobalRect
VDSetPlayThruDestination
VDSetPlayThruGlobalRect
 Video Clipping
VDClearClipRgn
VDGetClipState
VDSetClipRgn
VDSetClipState
*/

/*
QTVideoOutputCopyIndAudioOutputDeviceUID
QTVideoOutputGetIndImageDecompressor
VDGetInputGammaRecord
VDGetInputName
VDGetPreferredImageDimensions
VDIIDCGetCSRData
VDIIDCGetDefaultFeatures
VDIIDCGetFeatures
VDIIDCGetFeaturesForSpecifier
VDIIDCSetCSRData
VDIIDCSetFeatures
VDSetDestinationPort
VDSetInputGammaRecord
VDSetPreferredImageDimensions
VDUseSafeBuffers
*/

//void test ()
//{
//if ((i == count-1) && (inp == inputCount-1))
//{
//    OSG_NOTICE << "    * TEST SGSetChannelDevice(..) : " << pstr_printable(deviceNameRec.name) << std::endl;
//    result = SGSetChannelDevice (gVideoChannel, deviceNameStr);
//    if (result == noErr)
//    {
//        result = SGSetChannelDeviceInput( gVideoChannel, 0 );
//        result = SGGetSrcVideoBounds (gVideoChannel, &gActiveVideoRect);
//        OSG_NOTICE << "SrcVideoBounds: " << gActiveVideoRect.right << " " << gActiveVideoRect.bottom << std::endl;
//        Str255 deviceName2;
//        Str255 inputName2;
//        short  inputNumber2;
//        result = SGGetChannelDeviceAndInputNames( gVideoChannel, deviceName2, inputName2, &inputNumber2);
//        OSG_NOTICE << "ChannelDeviceAndInputNamesNumber: " << pstr_printable(deviceName2) << " : " << pstr_printable(inputName2) << " : " << inputNumber2 << std::endl;
//        result = SGGetChannelDeviceList( gVideoChannel, sgDeviceListIncludeInputs, &deviceList);
//        if (result != noErr)
//        {
//            OSG_NOTICE << "Could not get DeviceList from Video SG" << std::endl;
//        }
//        else
//        {
//            OSG_NOTICE << "DeviceList from Video SG ok" << std::endl;
//            short count         = (*deviceList)->count;
//            short selectedIndex = (*deviceList)->selectedIndex;
//            OSG_NOTICE << "DeviceList : " << count << " devices in total" << std::endl;
//            OSG_NOTICE << "DeviceList : " << selectedIndex << " is current device" << std::endl;
//        }
//    }
//    else
//    {
//        OSG_NOTICE << "SGSetChannelDevice - failed!" << std::endl;
//    }
//    OSG_NOTICE << "    * TEST SGSetChannelDevice(..) end" << std::endl;
//}
