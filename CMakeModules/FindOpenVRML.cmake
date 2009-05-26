# Locate openvml library
# This module defines
# OPENVRML_LIBRARY
# OPENVRML_FOUND, if false, do not try to link to vrml 
# OPENVRML_INCLUDE_DIR, where to find the headers
#
# $OPENVRML_DIR is an environment variable that would
# correspond to the ./configure --prefix=$OPENVRML_DIR
#
# Created by Robert Osfield. 
# Modified for the debug library by Jean-Sébastien Guay.

FIND_PATH(OPENVRML_INCLUDE_DIR openvrml/openvrml-common.h
    $ENV{OPENVRML_DIR}/include
    $ENV{OPENVRML_DIR}
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

FIND_LIBRARY(OPENVRML_LIBRARY 
    NAMES openvrml
    PATHS
    $ENV{OPENVRML_DIR}/lib
    $ENV{OPENVRML_DIR}
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

FIND_LIBRARY(OPENVRML_LIBRARY_DEBUG 
    NAMES openvrmld
    PATHS
    $ENV{OPENVRML_DIR}/lib
    $ENV{OPENVRML_DIR}
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

SET(OPENVRML_FOUND "NO")
IF(OPENVRML_LIBRARY AND OPENVRML_INCLUDE_DIR)
    SET(OPENVRML_FOUND "YES")
ENDIF(OPENVRML_LIBRARY AND OPENVRML_INCLUDE_DIR)


