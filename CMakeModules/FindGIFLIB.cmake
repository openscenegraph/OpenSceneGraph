# Locate gdal
# This module defines
# GIFLIB_LIBRARY
# GIFLIB_FOUND, if false, do not try to link to gdal 
# GIFLIB_INCLUDE_DIR, where to find the headers
#
# $GIFLIB_DIR is an environment variable that would
# correspond to the ./configure --prefix=$GIFLIB_DIR
# used in building gdal.
#
# Created by Eric Wing. 

FIND_PATH(GIFLIB_INCLUDE_DIR gif_lib.h
    $ENV{GIFLIB_DIR}/include
    $ENV{GIFLIB_DIR}
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

FIND_LIBRARY(GIFLIB_LIBRARY 
    NAMES gif ungif libgif libungif
    PATHS
    $ENV{GIFLIB_DIR}/lib
    $ENV{GIFLIB_DIR}
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

SET(GIFLIB_FOUND "NO")
IF(GIFLIB_LIBRARY AND GIFLIB_INCLUDE_DIR)
    SET(GIFLIB_FOUND "YES")
ENDIF(GIFLIB_LIBRARY AND GIFLIB_INCLUDE_DIR)


