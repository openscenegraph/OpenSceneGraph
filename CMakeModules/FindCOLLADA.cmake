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

SET(COLLADA_DOM_ROOT "$ENV{COLLADA_DIR}/dom" CACHE PATH "Location of Collada DOM directory")

FIND_PATH(COLLADA_INCLUDE_DIR dae.h
    ${COLLADA_DOM_ROOT}/include
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
    ${COLLADA_DOM_ROOT}/build/vc8-1.4
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
    ${COLLADA_DOM_ROOT}/build/vc8-1.4-d
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
    NAMES libcollada14dom21-s
    PATHS
    ${COLLADA_DOM_ROOT}/build/vc8-1.4
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
    ${COLLADA_DOM_ROOT}/build/vc8-1.4-d
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

IF(COLLADA_STATIC_LIBRARY)

    # find extra libraries that the static linking requires

    FIND_PACKAGE(LibXml2)
    IF (LIBXML2_FOUND)
        SET(COLLADA_LIBXML_LIBRARY ${LIBXML2_LIBRARIES})
    ELSE(LIBXML2_FOUND)
        IF(WIN32)
            FIND_LIBRARY(COLLADA_LIBXML_LIBRARY
                NAMES libxml2
                PATHS
                ${COLLADA_DOM_ROOT}/external-libs/libxml2/win32/lib
                ${COLLADA_DOM_ROOT}/external-libs/libxml2/mingw/lib
            )
        ENDIF(WIN32)
    ENDIF(LIBXML2_FOUND)
    
    FIND_PACKAGE(ZLIB)
    IF (ZLIB_FOUND)
        SET(COLLADA_ZLIB_LIBRARY ${ZLIB_LIBRARY})
    ELSE(ZLIB_FOUND)
        IF(WIN32)
            FIND_LIBRARY(COLLADA_ZLIB_LIBRARY
                NAMES zlib
                PATHS
                ${COLLADA_DOM_ROOT}/external-libs/libxml2/win32/lib
                ${COLLADA_DOM_ROOT}/external-libs/libxml2/mingw/lib
            )
        ENDIF(WIN32)
    ENDIF(ZLIB_FOUND)

    IF(WIN32)

        FIND_LIBRARY(COLLADA_PCRECPP_LIBRARY
            NAMES pcrecpp
            PATHS
            ${COLLADA_DOM_ROOT}/external-libs/pcre/lib/vc8    
            ${COLLADA_DOM_ROOT}/external-libs/pcre/lib/vc9
            ${COLLADA_DOM_ROOT}/external-libs/pcre/lib/mac
            ${COLLADA_DOM_ROOT}/external-libs/pcre/lib/mingw    
        )

        FIND_LIBRARY(COLLADA_PCRECPP_LIBRARY_DEBUG 
            NAMES pcrecpp-d
            PATHS
            ${COLLADA_DOM_ROOT}/external-libs/pcre/lib/vc8    
            ${COLLADA_DOM_ROOT}/external-libs/pcre/lib/vc9
            ${COLLADA_DOM_ROOT}/external-libs/pcre/lib/mac
            ${COLLADA_DOM_ROOT}/external-libs/pcre/lib/mingw    
        )

        FIND_LIBRARY(COLLADA_PCRE_LIBRARY
            NAMES pcre
            PATHS
            ${COLLADA_DOM_ROOT}/external-libs/pcre/lib/vc8    
            ${COLLADA_DOM_ROOT}/external-libs/pcre/lib/vc9
            ${COLLADA_DOM_ROOT}/external-libs/pcre/lib/mac
            ${COLLADA_DOM_ROOT}/external-libs/pcre/lib/mingw    
        )

        FIND_LIBRARY(COLLADA_PCRE_LIBRARY_DEBUG 
            NAMES pcre-d
            PATHS
            ${COLLADA_DOM_ROOT}/external-libs/pcre/lib/vc8    
            ${COLLADA_DOM_ROOT}/external-libs/pcre/lib/vc9
            ${COLLADA_DOM_ROOT}/external-libs/pcre/lib/mac
            ${COLLADA_DOM_ROOT}/external-libs/pcre/lib/mingw
        )

        FIND_LIBRARY(COLLADA_MINIZIP_LIBRARY
            NAMES minizip
            PATHS
            ${COLLADA_DOM_ROOT}/external-libs/minizip/win32/lib
            ${COLLADA_DOM_ROOT}/external-libs/minizip/mac
        )

        FIND_LIBRARY(COLLADA_MINIZIP_LIBRARY_DEBUG
            NAMES minizip-d
            PATHS
            ${COLLADA_DOM_ROOT}/external-libs/minizip/win32/lib
            ${COLLADA_DOM_ROOT}/external-libs/minizip/mac
        )
    
    ENDIF(WIN32)

ENDIF(COLLADA_STATIC_LIBRARY)

IF(COLLADA_DYNAMIC_LIBRARY OR COLLADA_STATIC_LIBRARY)

    IF(WIN32)
    
        FIND_LIBRARY(COLLADA_BOOST_FILESYSTEM_LIBRARY
            NAMES libboost_filesystem
            PATHS
            ${COLLADA_DOM_ROOT}/external-libs/boost/lib/vc8
            ${COLLADA_DOM_ROOT}/external-libs/boost/lib/vc9
            ${COLLADA_DOM_ROOT}/external-libs/boost/lib/mingw
        )

        FIND_LIBRARY(COLLADA_BOOST_FILESYSTEM_LIBRARY_DEBUG
            NAMES libboost_filesystem-d
            PATHS
            ${COLLADA_DOM_ROOT}/external-libs/boost/lib/vc8
            ${COLLADA_DOM_ROOT}/external-libs/boost/lib/vc9
            ${COLLADA_DOM_ROOT}/external-libs/boost/lib/mingw
        )

        FIND_LIBRARY(COLLADA_BOOST_SYSTEM_LIBRARY
            NAMES libboost_system
            PATHS
            ${COLLADA_DOM_ROOT}/external-libs/boost/lib/vc8
            ${COLLADA_DOM_ROOT}/external-libs/boost/lib/vc9
            ${COLLADA_DOM_ROOT}/external-libs/boost/lib/mingw
        )

        FIND_LIBRARY(COLLADA_BOOST_SYSTEM_LIBRARY_DEBUG
            NAMES libboost_system-d
            PATHS
            ${COLLADA_DOM_ROOT}/external-libs/boost/lib/vc8
            ${COLLADA_DOM_ROOT}/external-libs/boost/lib/vc9
            ${COLLADA_DOM_ROOT}/external-libs/boost/lib/mingw
        )

        SET(COLLADA_BOOST_INCLUDE_DIR ${COLLADA_DOM_ROOT}/external-libs/boost)

    ENDIF(WIN32)

ENDIF(COLLADA_DYNAMIC_LIBRARY OR COLLADA_STATIC_LIBRARY)

SET(COLLADA_FOUND "NO")
IF(COLLADA_DYNAMIC_LIBRARY OR COLLADA_STATIC_LIBRARY)
    IF   (COLLADA_INCLUDE_DIR)
        SET(COLLADA_FOUND "YES")
    ENDIF(COLLADA_INCLUDE_DIR)
ENDIF(COLLADA_DYNAMIC_LIBRARY OR COLLADA_STATIC_LIBRARY)
