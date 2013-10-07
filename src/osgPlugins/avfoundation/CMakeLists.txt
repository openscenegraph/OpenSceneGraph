
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
