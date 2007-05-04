# Locate gdal
# This module defines
# JASPER_LIBRARY
# JASPER_FOUND, if false, do not try to link to gdal 
# JASPER_INCLUDE_DIR, where to find the headers
#
# $JASPER_DIR is an environment variable that would
# correspond to the ./configure --prefix=$JASPER_DIR
#
# Created by Robert Osfield. 

FIND_PATH(JASPER_INCLUDE_DIR jasper/jasper.h
    $ENV{JASPER_DIR}/include
    $ENV{JASPER_DIR}
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

FIND_LIBRARY(JASPER_LIBRARY 
    NAMES jasper
    PATHS
    $ENV{JASPER_DIR}/lib
    $ENV{JASPER_DIR}
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

SET(JASPER_FOUND "NO")
IF(JASPER_LIBRARY AND JASPER_INCLUDE_DIR)
    SET(JASPER_FOUND "YES")
ENDIF(JASPER_LIBRARY AND JASPER_INCLUDE_DIR)


