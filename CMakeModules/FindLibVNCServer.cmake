# Locate libvncserver
# This module defines
# LIBVNCSERVER_LIBRARY
# LIBVNCSERVER_FOUND, if false, do not try to link to libvncserver 
# LIBVNCSERVER_INCLUDE_DIR, where to find the headers
#
# $LIBVNCSERVER_DIR is an environment variable that would
# correspond to the ./configure --prefix=$LIBVNCSERVER_DIR
# used in building libvncserver.

FIND_PATH(LIBVNCSERVER_INCLUDE_DIR rfb/rfb.h
    $ENV{LIBVNCSERVER_DIR}/include
    $ENV{LIBVNCSERVER_DIR}
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

FIND_LIBRARY(LIBVNCCLIENT_LIBRARY 
    NAMES vncclient
    PATHS
    $ENV{LIBVNCSERVER_DIR}/lib
    $ENV{LIBVNCSERVER_DIR}
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

FIND_LIBRARY(LIBVNCSERVER_LIBRARY 
    NAMES vncserver
    PATHS
    $ENV{LIBVNCSERVER_DIR}/lib
    $ENV{LIBVNCSERVER_DIR}
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

SET(LIBVNCSERVER_FOUND "NO")
IF(LIBVNCSERVER_LIBRARY AND LIBVNCSERVER_INCLUDE_DIR)
    SET(LIBVNCSERVER_FOUND "YES")
ENDIF(LIBVNCSERVER_LIBRARY AND LIBVNCSERVER_INCLUDE_DIR)


