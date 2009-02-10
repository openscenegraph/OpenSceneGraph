# Locate zlib
# This module defines
# ZLIB_LIBRARY
# ZLIB_FOUND, if false, do not try to link to zlib 
# ZLIB_INCLUDE_DIR, where to find the headers
#
# $ZLIB_DIR is an environment variable that would
# correspond to the ./configure --prefix=$ZLIB_DIR
# used in building zlib.
#
# Created by Ulrich Hertlein. 

FIND_PATH(ZLIB_INCLUDE_DIR zlib.h
    $ENV{ZLIB_DIR}/include
    $ENV{ZLIB_DIR}
    $ENV{OSGDIR}/include
    $ENV{OSGDIR}
    $ENV{OSG_ROOT}/include
    ~/Library/Frameworks
    /Library/Frameworks
    /usr/local/include
    /usr/include
    /sw/include # Fink
    /opt/local/include # DarwinPorts
    /opt/csw/include # Blastwave
    /opt/include
    [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;OSG_ROOT]/include
    /usr/freeware/include
)

FIND_LIBRARY(ZLIB_LIBRARY 
    NAMES z libz zlib
    PATHS
    $ENV{ZLIB_DIR}/lib
    $ENV{ZLIB_DIR}
    $ENV{OSGDIR}/lib
    $ENV{OSGDIR}
    $ENV{OSG_ROOT}/lib
    ~/Library/Frameworks
    /Library/Frameworks
    /usr/local/lib
    /usr/lib
    /sw/lib
    /opt/local/lib
    /opt/csw/lib
    /opt/lib
    [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;OSG_ROOT]/lib
    /usr/freeware/lib64
)

SET(ZLIB_FOUND "NO")
IF(ZLIB_LIBRARY AND ZLIB_INCLUDE_DIR)
    SET(ZLIB_FOUND "YES")
ENDIF(ZLIB_LIBRARY AND ZLIB_INCLUDE_DIR)


