# Locate QuickTime
# This module defines
# QUICKTIME_LIBRARY
# QUICKTIME_FOUND, if false, do not try to link to gdal
# QUICKTIME_INCLUDE_DIR, where to find the headers
#
# $QUICKTIME_DIR is an environment variable that would
# correspond to the ./configure --prefix=$QUICKTIME_DIR
#
# Created by Eric Wing.


# QuickTime on OS X looks different than QuickTime for Windows,
# so I am going to case the two.

IF(APPLE)
  FIND_PATH(QUICKTIME_INCLUDE_DIR QuickTime/QuickTime.h)
  FIND_LIBRARY(QUICKTIME_LIBRARY QuickTime)
ELSE()
  FIND_PATH(QUICKTIME_INCLUDE_DIR QuickTime.h
    $ENV{QUICKTIME_DIR}/include
    $ENV{QUICKTIME_DIR}
    NO_DEFAULT_PATH
  )
  FIND_PATH(QUICKTIME_INCLUDE_DIR QuickTime.h
    PATHS ${CMAKE_PREFIX_PATH} # Unofficial: We are proposing this.
    NO_DEFAULT_PATH
    PATH_SUFFIXES include
  )
  FIND_PATH(QUICKTIME_INCLUDE_DIR QuickTime.h)

  FIND_LIBRARY(QUICKTIME_LIBRARY QuickTime
    $ENV{QUICKTIME_DIR}/lib
    $ENV{QUICKTIME_DIR}
    NO_DEFAULT_PATH
  )
  FIND_LIBRARY(QUICKTIME_LIBRARY QuickTime
    PATHS ${CMAKE_PREFIX_PATH} # Unofficial: We are proposing this.
    NO_DEFAULT_PATH
    PATH_SUFFIXES lib64 lib
  )
  FIND_LIBRARY(QUICKTIME_LIBRARY QuickTime)
ENDIF()


SET(QUICKTIME_FOUND "NO")
IF(QUICKTIME_LIBRARY AND QUICKTIME_INCLUDE_DIR)
  SET(QUICKTIME_FOUND "YES")
ENDIF()

IF(OSG_BUILD_PLATFORM_IPHONE OR OSG_BUILD_PLATFORM_IPHONE_SIMULATOR)
    SET(QUICKTIME_FOUND "NO")
ELSE()
  IF(APPLE)
      #Quicktime is not supported under 64bit OSX build so we need to detect it and disable it.
      #First check to see if we are running with a native 64-bit compiler (10.6 default) and implicit arch
      IF(NOT CMAKE_OSX_ARCHITECTURES AND CMAKE_SIZEOF_VOID_P EQUAL 8)
          SET(QUICKTIME_FOUND "NO")
      ELSE()
          #Otherwise check to see if 64-bit is explicitly called for.
          LIST(FIND CMAKE_OSX_ARCHITECTURES "x86_64" has64Compile)
          IF(NOT has64Compile EQUAL -1)
              SET(QUICKTIME_FOUND "NO")
          ENDIF()
      ENDIF()
      # Disable quicktime for >= 10.7, as it's officially deprecated

      IF(${OSG_OSX_SDK_NAME} STREQUAL "macosx10.7" OR ${OSG_OSX_SDK_NAME} STREQUAL "macosx10.8" OR ${OSG_OSX_SDK_NAME} STREQUAL "macosx10.9" OR ${OSG_OSX_SDK_NAME} STREQUAL "macosx10.10")
          MESSAGE("disabling quicktime because it's not supported by the selected SDK ${OSG_OSX_SDK_NAME}")
          SET(QUICKTIME_FOUND "NO")
      ENDIF()
  ENDIF()
ENDIF()
