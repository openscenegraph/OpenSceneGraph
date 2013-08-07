# Locate V8
# This module defines
# V8_LIBRARY
# V8_FOUND, if false, do not try to link to gdal
# V8_INCLUDE_DIR, where to find the headers
#
# $V8_DIR is an environment variable that would
# correspond to the ./configure --prefix=$V8_DIR
#
# Created by Robert Osfield (based on FindFLTK.cmake)

FIND_PATH(V8_INCLUDE_DIR v8.h
    $ENV{V8_DIR}/include
    $ENV{V8_DIR}
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

FIND_LIBRARY(V8_LIBRARY
    NAMES v8 libv8
    PATHS
    $ENV{V8_DIR}/lib
    $ENV{V8_DIR}
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

SET(V8_FOUND "NO")
IF(V8_LIBRARY AND V8_INCLUDE_DIR)
    SET(V8_FOUND "YES")
ENDIF()
