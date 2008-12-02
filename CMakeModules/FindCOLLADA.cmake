# Locate Collada
# This module defines:
# COLLADA_INCLUDE_DIR, where to find the headers
#
# COLLADA_LIBRARY, COLLADA_LIBRARY_DEBUG
# COLLADA_FOUND, if false, do not try to link to Collada dynamically
#
# COLLADA_LIBRARY_STATIC, COLLADA_LIBRARY_STATIC_DEBUG
# COLLADA_STATIC_FOUND, if false, do not try to link to Collada statically
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

FIND_LIBRARY(COLLADA_DYNAMIC_LIBRARY 
    NAMES collada_dom collada14dom libcollada14dom21
    PATHS
    $ENV{COLLADA_DIR}/build/vc8-1.4
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

FIND_LIBRARY(COLLADA_DYNAMIC_LIBRARY_DEBUG 
    NAMES collada_dom-d collada14dom-d libcollada14dom21-d
    PATHS
    $ENV{COLLADA_DIR}/build/vc8-1.4-d
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

FIND_LIBRARY(COLLADA_STATIC_LIBRARY
    NAMES collada_dom-s collada14dom-s libcollada14dom21-s
    PATHS
    $ENV{COLLADA_DIR}/build/vc8-1.4
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

FIND_LIBRARY(COLLADA_STATIC_LIBRARY_DEBUG 
    NAMES collada_dom-sd collada14dom-sd libcollada14dom21-sd
    PATHS
    $ENV{COLLADA_DIR}/build/vc8-1.4-d
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
IF(COLLADA_DYNAMIC_LIBRARY OR COLLADA_STATIC_LIBRARY)
    IF   (COLLADA_INCLUDE_DIR)
        SET(COLLADA_FOUND "YES")
    ENDIF(COLLADA_INCLUDE_DIR)
ENDIF(COLLADA_DYNAMIC_LIBRARY OR COLLADA_STATIC_LIBRARY)
