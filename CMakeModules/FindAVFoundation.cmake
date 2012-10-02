# Locate Apple AVFoundation (next-generation QTKit)
# This module defines
# AV_FOUNDATION_LIBRARY
# AV_FOUNDATION_FOUND, if false, do not try to link to gdal 
# AV_FOUNDATION_INCLUDE_DIR, where to find the headers
#
# $AV_FOUNDATION_DIR is an environment variable that would
# correspond to the ./configure --prefix=$AV_FOUNDATION_DIR
#
# Created by Stephan Maximilian Huber 

# QTKit on OS X looks different than QTKit for Windows,
# so I am going to case the two.

IF(APPLE)
  FIND_PATH(AV_FOUNDATION_INCLUDE_DIR AVFoundation/AVFoundation.h)
  FIND_LIBRARY(AV_FOUNDATION_LIBRARY AVFoundation)
ENDIF()

SET(AV_FOUNDATION_FOUND "NO")
IF(AV_FOUNDATION_LIBRARY AND AV_FOUNDATION_INCLUDE_DIR)
  SET(AV_FOUNDATION_FOUND "YES")
ENDIF()

IF(OSG_BUILD_PLATFORM_IPHONE OR OSG_BUILD_PLATFORM_IPHONE_SIMULATOR)
    # TODO, AVFoundation exists ON iOS, too
    SET(AV_FOUNDATION_FOUND "NO")
ENDIF()

IF(APPLE)
    # AVFoundation exists since 10.7, but only 10.8 has all features necessary for OSG
    # so check the SDK-setting

    IF(${OSG_OSX_SDK_NAME} STREQUAL "macosx10.8")
        # nothing special here ;-)
    ELSE()
        MESSAGE("AVFoundation disabled for SDK < 10.8")
        SET(AV_FOUNDATION_FOUND "NO")  
    ENDIF()
ENDIF()

