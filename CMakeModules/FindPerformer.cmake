# Locate Performer
# This module defines
# PERFORMER_LIBRARY
# PERFORMER_FOUND, if false, do not try to link to gdal 
# PERFORMER_INCLUDE_DIR, where to find the headers
#
# $PERFORMER_DIR is an environment variable that would
# correspond to the ./configure --prefix=$PERFORMER_DIR
#
# Created by Robert Osfield. 

FIND_PATH(PERFORMER_INCLUDE_DIR Performer/pfdu.h
    $ENV{PFROOT}/include
    $ENV{PFROOT}
    $ENV{PERFORMER_DIR}/include
    $ENV{PERFORMER_DIR}
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

IF(MSVC)
    FIND_LIBRARY(PERFORMER_LIBRARY 
        NAMES libpf
        PATHS
        $ENV{PFROOT}/lib
        $ENV{PFROOT}
        $ENV{PERFORMER_DIR}/lib
        $ENV{PERFORMER_DIR}
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
ELSE(MSVC)
    FIND_LIBRARY(PERFORMER_LIBRARY 
        NAMES pf
        PATHS
        $ENV{PFROOT}/lib
        $ENV{PFROOT}
        $ENV{PERFORMER_DIR}/lib
        $ENV{PERFORMER_DIR}
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
ENDIF(MSVC)

SET(PERFORMER_FOUND "NO")
IF(PERFORMER_LIBRARY AND PERFORMER_INCLUDE_DIR)
    SET(PERFORMER_FOUND "YES")
ENDIF(PERFORMER_LIBRARY AND PERFORMER_INCLUDE_DIR)


