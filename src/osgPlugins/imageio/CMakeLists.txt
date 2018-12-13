SET(TARGET_SRC ReaderWriterImageIO.cpp )

IF (APPLE) 
   IF(OSG_BUILD_PLATFORM_IPHONE)
     # compile as objective-c++
     SET_SOURCE_FILES_PROPERTIES(ReaderWriterImageIO.cpp PROPERTIES COMPILE_FLAGS "-x objective-c++")
   ENDIF()
ENDIF()

SET(TARGET_ADDED_LIBRARIES)
SET(TARGET_LIBRARIES_VARS IMAGEIO_LIBRARY )
SET(TARGET_EXTERNAL_LIBRARIES "/System/Library/Frameworks/Accelerate.framework" )

#### end var setup  ###
SETUP_PLUGIN(imageio)

