# Locate Apple CoreMedia
# This module defines
# COREMEDIA_LIBRARY
# COREMEDIA_FOUND, if false, do not try to link to gdal 
# COREMEDIA_INCLUDE_DIR, where to find the headers
#
# $COREMEDIA_DIR is an environment variable that would
# correspond to the ./configure --prefix=$COREMEDIA_DIR
#
# Created by Stephan Maximilian Huber. 


IF(APPLE)
  FIND_PATH(COREMEDIA_INCLUDE_DIR CoreMedia/CoreMedia.h)
  FIND_LIBRARY(COREMEDIA_LIBRARY CoreMedia)
ENDIF()


SET(COREMEDIA_FOUND "NO")
IF(COREMEDIA_LIBRARY AND COREMEDIA_INCLUDE_DIR)
  SET(COREMEDIA_FOUND "YES")
ENDIF()

