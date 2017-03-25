# AVFoundation plugin only works with OSX/Cocoa (not X11 or Carbon)
IF(NOT OSG_WINDOWING_SYSTEM STREQUAL "Cocoa" AND NOT OSG_WINDOWING_SYSTEM STREQUAL "IOS")
  MESSAGE(WARNING "Disabling AVFoundation plugin because it requires OSG_WINDOWING_SYSTEM to be Cocoa or IOS")
  RETURN()
ENDIF()

SET(TARGET_SRC
    OSXAVFoundationVideo.mm
    OSXAVFoundationVideo.h
    ../QTKit/VideoFrameDispatcher.h
    ../QTKit/VideoFrameDispatcher.cpp
    OSXAVFoundationCoreVideoTexture.h
    OSXAVFoundationCoreVideoTexture.cpp
    ReaderWriterAVFoundation.cpp
)

SET(TARGET_LIBRARIES_VARS AV_FOUNDATION_LIBRARY COCOA_LIBRARY COREVIDEO_LIBRARY COREMEDIA_LIBRARY QUARTZCORE_LIBRARY)
SET(TARGET_ADDED_LIBRARIES osgViewer )

#### end var setup  ###
SETUP_PLUGIN(avfoundation)
