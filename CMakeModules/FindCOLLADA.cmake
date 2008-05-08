# Locate gdal
# This module defines
# COLLADA_LIBRARY
# COLLADA_FOUND, if false, do not try to link to gdal 
# COLLADA_INCLUDE_DIR, where to find the headers
#
# $COLLADA_DIR is an environment variable that would
# correspond to the ./configure --prefix=$COLLADA_DIR
#
# Created by Robert Osfield. 

FIND_PATH(COLLADA_INCLUDE_DIR dae.h
    $ENV{COLLADA_DIR}/include
    $ENV{COLLADA_DIR}
    $ENV{OSGDIR}/include
    $ENV{OSGDIR}
    $ENV{OSG_ROOT}/include
    ~/Library/Frameworks
    /Library/Frameworks
    /usr/local/include
    /usr/local/include/colladadom
    /usr/include/
    /usr/include/colladadom
    /sw/include # Fink
    /opt/local/include # DarwinPorts
    /opt/csw/include # Blastwave
    /opt/include
    [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;OSG_ROOT]/include
    /usr/freeware/include
)

FIND_LIBRARY(COLLADA_LIBRARY 
    NAMES collada_dom collada14dom
    PATHS
    $ENV{COLLADA_DIR}/lib
    $ENV{COLLADA_DIR}/lib-dbg
    $ENV{COLLADA_DIR}
    $ENV{OSGDIR}/lib
    $ENV{OSGDIR}
    $ENV{OSG_ROOT}/lib
    ~/Library/Frameworks
    /Library/Frameworks
    /usr/local/lib
    /usr/local/lib64
    /usr/lib
    /usr/lib64
    /sw/lib
    /opt/local/lib
    /opt/csw/lib
    /opt/lib
    [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;OSG_ROOT]/lib
    /usr/freeware/lib64
)

SET(COLLADA_FOUND "NO")
IF(COLLADA_LIBRARY AND COLLADA_INCLUDE_DIR)
    SET(COLLADA_FOUND "YES")
ENDIF(COLLADA_LIBRARY AND COLLADA_INCLUDE_DIR)


