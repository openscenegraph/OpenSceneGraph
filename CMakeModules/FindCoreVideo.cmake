# Locate Apple CoreVideo (next-generation QuickTime)
# This module defines
# COREVIDEO_LIBRARY
# COREVIDEO_FOUND, if false, do not try to link to gdal 
# COREVIDEO_INCLUDE_DIR, where to find the headers
#
# $COREVIDEO_DIR is an environment variable that would
# correspond to the ./configure --prefix=$COREVIDEO_DIR
#
# Created by Eric Wing. 

# CoreVideo on OS X looks different than CoreVideo for Windows,
# so I am going to case the two.

IF(APPLE)
  IF(OSG_BUILD_PLATFORM_IPHONE)
    FIND_PATH(COREVIDEO_INCLUDE_DIR CoreVideo/CoreVideo.h PATHS ${IPHONE_SDKROOT}/System/Library PATH_SUFFIXES Frameworks NO_DEFAULT_PATH)
    FIND_LIBRARY(COREVIDEO_LIBRARY CoreVideo PATHS ${IPHONE_SDKROOT}/System/Library PATH_SUFFIXES Frameworks NO_DEFAULT_PATH)
  ELSE()
    FIND_PATH(COREVIDEO_INCLUDE_DIR CoreVideo/CoreVideo.h)
    FIND_LIBRARY(COREVIDEO_LIBRARY CoreVideo)
  ENDIF()
ENDIF()


SET(COREVIDEO_FOUND "NO")
IF(COREVIDEO_LIBRARY AND COREVIDEO_INCLUDE_DIR)
  SET(COREVIDEO_FOUND "YES")
ENDIF()

