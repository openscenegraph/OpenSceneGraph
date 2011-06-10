# Locate OpenEXR
# This module defines
# OPENEXR_LIBRARY
# OPENEXR_FOUND, if false, do not try to link to OpenEXR 
# OPENEXR_INCLUDE_DIR, where to find the headers
#
# $OPENEXR_DIR is an environment variable that would
# correspond to the ./configure --prefix=$OPENEXR_DIR
#
# Created by Robert Osfield. 

FIND_PATH(OPENEXR_INCLUDE_DIR OpenEXR/ImfIO.h
    $ENV{OPENEXR_DIR}/include
    $ENV{OPENEXR_DIR}
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

FIND_LIBRARY(OPENEXR_IlmIlf_LIBRARY 
    NAMES IlmImf
    PATHS
    $ENV{OPENEXR_DIR}/lib
    $ENV{OPENEXR_DIR}
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

FIND_LIBRARY(OPENEXR_IlmThread_LIBRARY 
    NAMES IlmThread
    PATHS
    $ENV{OPENEXR_DIR}/lib
    $ENV{OPENEXR_DIR}
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


FIND_LIBRARY(OPENEXR_Iex_LIBRARY 
    NAMES Iex
    PATHS
    $ENV{OPENEXR_DIR}/lib
    $ENV{OPENEXR_DIR}
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

FIND_LIBRARY(OPENEXR_Half_LIBRARY 
    NAMES Half
    PATHS
    $ENV{OPENEXR_DIR}/lib
    $ENV{OPENEXR_DIR}
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

SET(OPENEXR_FOUND "NO")
IF(OPENEXR_INCLUDE_DIR AND OPENEXR_IlmIlf_LIBRARY AND OPENEXR_IlmThread_LIBRARY AND OPENEXR_Iex_LIBRARY AND OPENEXR_Half_LIBRARY)
    SET(OPENEXR_LIBRARIES
        ${OPENEXR_IlmIlf_LIBRARY}
        ${OPENEXR_IlmThread_LIBRARY}
        ${OPENEXR_Half_LIBRARY}
        ${OPENEXR_Iex_LIBRARY}
    )
    SET(OPENEXR_FOUND "YES")
ENDIF(OPENEXR_INCLUDE_DIR AND OPENEXR_IlmIlf_LIBRARY AND OPENEXR_IlmThread_LIBRARY AND OPENEXR_Iex_LIBRARY AND OPENEXR_Half_LIBRARY)
