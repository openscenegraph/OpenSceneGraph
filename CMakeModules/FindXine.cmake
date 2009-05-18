# Locate gdal
# This module defines
# XINE_LIBRARY
# XINE_FOUND, if false, do not try to link to gdal 
# XINE_INCLUDE_DIR, where to find the headers
#
# $XINE_DIR is an environment variable that would
# correspond to the ./configure --prefix=$XINE_DIR
#
# Created by Robert Osfield. 

FIND_PATH(XINE_INCLUDE_DIR xine.h
    $ENV{XINE_DIR}/include
    $ENV{XINE_DIR}
    ~/Library/Frameworks
    /Library/Frameworks
    /usr/local/include
    /usr/include
    /sw/include # Fink
    /opt/local/include # DarwinPorts
    /opt/csw/include # Blastwave
    /opt/include
    /usr/freeware/include
)

FIND_LIBRARY(XINE_LIBRARY 
    NAMES xine
    PATHS
    $ENV{XINE_DIR}/lib
    $ENV{XINE_DIR}
    ~/Library/Frameworks
    /Library/Frameworks
    /usr/local/lib
    /usr/lib
    /sw/lib
    /opt/local/lib
    /opt/csw/lib
    /opt/lib
    /usr/freeware/lib64
)

SET(XINE_FOUND "NO")
IF(XINE_LIBRARY AND XINE_INCLUDE_DIR)
    SET(XINE_FOUND "YES")
ENDIF(XINE_LIBRARY AND XINE_INCLUDE_DIR)


