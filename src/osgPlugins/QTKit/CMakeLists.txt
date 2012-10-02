INCLUDE_DIRECTORIES( ${QTKIT_INCLUDE_DIR} )

SET(TARGET_SRC 
    ReaderWriterQTKit.cpp
    OSXQTKitVideo.h
    OSXQTKitVideo.mm
    OSXCoreVideoAdapter.h
    OSXCoreVideoAdapter.mm
    OSXCoreVideoTexture.cpp
    OSXCoreVideoTexture.h
    VideoFrameDispatcher.h
    VideoFrameDispatcher.cpp
)

SET(TARGET_LIBRARIES_VARS QTKIT_LIBRARY COCOA_LIBRARY QUICKTIME_LIBRARY COREVIDEO_LIBRARY)
SET(TARGET_ADDED_LIBRARIES osgViewer )


IF(CMAKE_COMPILER_IS_GNUCXX)
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-deprecated-declarations")
ENDIF()


#### end var setup  ###
SETUP_PLUGIN(QTKit)
