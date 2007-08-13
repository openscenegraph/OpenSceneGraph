# Locate gdal
# This module defines
# GDAL_LIBRARY
# GDAL_FOUND, if false, do not try to link to gdal 
# GDAL_INCLUDE_DIR, where to find the headers
#
# $GDAL_DIR is an environment variable that would
# correspond to the ./configure --prefix=$GDAL_DIR
#
# Created by Robert Osfield. 

FIND_PATH(GDAL_INCLUDE_DIR gdal.h
    ${GDAL_DIR}/include
    $ENV{GDAL_DIR}/include
    $ENV{GDAL_DIR}
    $ENV{OSGDIR}/include
    $ENV{OSGDIR}
    $ENV{OSG_ROOT}/include
    ~/Library/Frameworks
    /Library/Frameworks
    /usr/local/include
    /usr/include
    /usr/include/gdal
    /sw/include # Fink
    /opt/local/include # DarwinPorts
    /opt/csw/include # Blastwave
    /opt/include
    [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;OSG_ROOT]/include
    /usr/freeware/include
)

FIND_LIBRARY(GDAL_LIBRARY 
    NAMES gdal gdal_i gdal1.4.0 gdal1.3.2
    PATHS
    ${GDAL_DIR}/lib
    $ENV{GDAL_DIR}/lib
    $ENV{GDAL_DIR}
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

SET(GDAL_FOUND "NO")
IF(GDAL_LIBRARY AND GDAL_INCLUDE_DIR)
    SET(GDAL_FOUND "YES")
ENDIF(GDAL_LIBRARY AND GDAL_INCLUDE_DIR)


