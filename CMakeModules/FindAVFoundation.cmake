# Locate Apple AVFoundation (next-generation QTKit)
# This module defines
# AV_FOUNDATION_LIBRARY
# AV_FOUNDATION_FOUND, if false, do not try to link to gdal
#
# $AV_FOUNDATION_DIR is an environment variable that would
# correspond to the ./configure --prefix=$AV_FOUNDATION_DIR
#
# Created by Stephan Maximilian Huber


IF(APPLE)
  IF(OSG_BUILD_PLATFORM_IPHONE)
    FIND_LIBRARY(AV_FOUNDATION_LIBRARY AVFoundation PATHS ${IPHONE_SDKROOT}/System/Library PATH_SUFFIXES Frameworks NO_DEFAULT_PATH)
  ELSE()
    FIND_LIBRARY(AV_FOUNDATION_LIBRARY AVFoundation)
  ENDIF()
ENDIF()

SET(AV_FOUNDATION_FOUND "NO")
IF(AV_FOUNDATION_LIBRARY)
  SET(AV_FOUNDATION_FOUND "YES")
ENDIF()

IF(OSG_BUILD_PLATFORM_IPHONE OR OSG_BUILD_PLATFORM_IPHONE_SIMULATOR)
    # AVFoundation exists ON iOS, too -- good support for SDK 6.0 and greater
    IF(${IPHONE_SDKVER} LESS "6.0")
        SET(AV_FOUNDATION_FOUND "NO")
    ELSE()
        SET(AV_FOUNDATION_FOUND "YES")
    ENDIF()
ELSE()
  IF(APPLE)
      # AVFoundation exists since 10.7, but only 10.8 has all features necessary for OSG
      # so check the SDK-setting

      IF(OSG_OSX_VERSION VERSION_LESS 10.8)
        MESSAGE("AVFoundation disabled for SDK < 10.8")
        SET(AV_FOUNDATION_FOUND "NO")
      ENDIF()
  ENDIF()
ENDIF()
