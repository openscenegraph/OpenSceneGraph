# Locate Apple QTKit (next-generation QuickTime)
# This module defines
# QTKIT_LIBRARY
# QTKIT_FOUND, if false, do not try to link to gdal 
# QTKIT_INCLUDE_DIR, where to find the headers
#
# $QTKIT_DIR is an environment variable that would
# correspond to the ./configure --prefix=$QTKIT_DIR
#
# Created by Eric Wing. 

# QTKit on OS X looks different than QTKit for Windows,
# so I am going to case the two.

IF(APPLE)
  FIND_PATH(QTKIT_INCLUDE_DIR QTKit/QTKit.h)
  FIND_LIBRARY(QTKIT_LIBRARY QTKit)
ENDIF()


SET(QTKIT_FOUND "NO")
IF(QTKIT_LIBRARY AND QTKIT_INCLUDE_DIR)
  SET(QTKIT_FOUND "YES")
ENDIF()

IF(OSG_BUILD_PLATFORM_IPHONE OR OSG_BUILD_PLATFORM_IPHONE_SIMULATOR)
    SET(QTKIT_FOUND "NO")
ENDIF()

IF(APPLE)
    # Technically QTKit is 64-bit capable, but the QTKit plug-in currently uses
    # a few 32-bit only APIs to bridge QTKit and Core Video.
    # As such, the plugin won't compile for 64-bit until Apple fixes this hole 
    # in their API.
    # For simplicitly, I pretend QTKit is only 32-bit, but if/when Apple fixes
    # this, we need an OS version check.
    # Snow Leopard still lacks a 64-bit path for this.
    #First check to see if we are running with a native 64-bit compiler (10.6 default) and implicit arch
    IF(NOT CMAKE_OSX_ARCHITECTURES AND CMAKE_SIZEOF_VOID_P EQUAL 8)
        SET(QTKIT_FOUND "NO")
    ELSE()
        #Otherwise check to see if 64-bit is explicitly called for.
        LIST(FIND CMAKE_OSX_ARCHITECTURES "x86_64" has64Compile)
        IF(NOT has64Compile EQUAL -1)
            SET(QTKIT_FOUND "NO")
        ENDIF()
    ENDIF()
ENDIF()

