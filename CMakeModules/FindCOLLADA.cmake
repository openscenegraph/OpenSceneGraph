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


# Check if COLLADA_DIR is set, otherwise use ACTUAL_3DPARTY_DIR:
SET( COLLADA_ENV_VAR_AVAILABLE $ENV{COLLADA_DIR} )
IF ( COLLADA_ENV_VAR_AVAILABLE )
    SET(COLLADA_DOM_ROOT "$ENV{COLLADA_DIR}/dom" CACHE PATH "Location of Collada DOM directory" FORCE)
ELSE ( COLLADA_ENV_VAR_AVAILABLE )
    SET(COLLADA_DOM_ROOT "${ACTUAL_3DPARTY_DIR}/include/1.4/dom" CACHE PATH "Location of Collada DOM directory" FORCE)
ENDIF( COLLADA_ENV_VAR_AVAILABLE )



IF(APPLE)
    SET(COLLADA_BUILDNAME "mac")
ELSEIF(MINGW)
    SET(COLLADA_BUILDNAME "mingw")
ELSEIF(MSVC12)
    SET(COLLADA_BUILDNAME "vc12")
ELSEIF(MSVC11)
    SET(COLLADA_BUILDNAME "vc11")
ELSEIF(MSVC10)
    SET(COLLADA_BUILDNAME "vc10")
ELSEIF(MSVC90)
    SET(COLLADA_BUILDNAME "vc9")
ELSEIF(MSVC80)
    SET(COLLADA_BUILDNAME "vc8")
ELSE(APPLE)
  SET(COLLADA_BUILDNAME "linux")
ENDIF(APPLE)


FIND_PATH(COLLADA_INCLUDE_DIR dae.h
    ${COLLADA_DOM_ROOT}/include
    $ENV{COLLADA_DIR}/include
    $ENV{COLLADA_DIR}
    ~/Library/Frameworks
    /Library/Frameworks
    /opt/local/Library/Frameworks #macports
    /usr/local/include
    /usr/local/include/colladadom
    /usr/local/include/collada-dom
    /opt/local/include/collada-dom
    /usr/include/
    /usr/include/colladadom
    /usr/include/collada-dom
    /sw/include # Fink
    /opt/local/include # DarwinPorts
    /opt/csw/include # Blastwave
    /opt/include
    /usr/freeware/include
    ${ACTUAL_3DPARTY_DIR}/include
)

FIND_LIBRARY(COLLADA_DYNAMIC_LIBRARY 
    NAMES collada_dom collada14dom Collada14Dom libcollada14dom21 libcollada14dom22
    PATHS
    ${COLLADA_DOM_ROOT}/build/${COLLADA_BUILDNAME}-1.4
    $ENV{COLLADA_DIR}/build/${COLLADA_BUILDNAME}-1.4
    $ENV{COLLADA_DIR}/lib
    $ENV{COLLADA_DIR}/lib-dbg
    $ENV{COLLADA_DIR}
    ~/Library/Frameworks
    /Library/Frameworks    
    /opt/local/Library/Frameworks #macports
    /usr/local/lib
    /usr/local/lib64
    /usr/lib
    /usr/lib64
    /sw/lib
    /opt/local/lib
    /opt/csw/lib
    /opt/lib
    /usr/freeware/lib64
    ${ACTUAL_3DPARTY_DIR}/lib
)

FIND_LIBRARY(COLLADA_DYNAMIC_LIBRARY_DEBUG 
    NAMES collada_dom-d collada14dom-d Collada14Dom-d libcollada14dom21-d libcollada14dom22-d
    PATHS
    ${COLLADA_DOM_ROOT}/build/${COLLADA_BUILDNAME}-1.4-d
    $ENV{COLLADA_DIR}/build/${COLLADA_BUILDNAME}-1.4-d
    $ENV{COLLADA_DIR}/lib
    $ENV{COLLADA_DIR}/lib-dbg
    $ENV{COLLADA_DIR}
    ~/Library/Frameworks
    /Library/Frameworks
    /opt/local/Library/Frameworks #macports
    /usr/local/lib
    /usr/local/lib64
    /usr/lib
    /usr/lib64
    /sw/lib
    /opt/local/lib
    /opt/csw/lib
    /opt/lib
    /usr/freeware/lib64
    ${ACTUAL_3DPARTY_DIR}/lib
)

FIND_LIBRARY(COLLADA_STATIC_LIBRARY 
    NAMES libcollada14dom21-s  libcollada14dom22-s libcollada14dom.a
    PATHS
    ${COLLADA_DOM_ROOT}/build/${COLLADA_BUILDNAME}-1.4
    $ENV{COLLADA_DIR}/build/${COLLADA_BUILDNAME}-1.4
    $ENV{COLLADA_DIR}/lib
    $ENV{COLLADA_DIR}/lib-dbg
    $ENV{COLLADA_DIR}
    ~/Library/Frameworks
    /Library/Frameworks
    /opt/local/Library/Frameworks #macports
    /usr/local/lib
    /usr/local/lib64
    /usr/lib
    /usr/lib64
    /sw/lib
    /opt/local/lib
    /opt/csw/lib
    /opt/lib
    /usr/freeware/lib64
    ${ACTUAL_3DPARTY_DIR}/lib
)

FIND_LIBRARY(COLLADA_STATIC_LIBRARY_DEBUG 
    NAMES collada_dom-sd collada14dom-sd libcollada14dom21-sd libcollada14dom22-sd libcollada14dom-d.a
    PATHS
    ${COLLADA_DOM_ROOT}/build/${COLLADA_BUILDNAME}-1.4-d
    $ENV{COLLADA_DIR}/build/${COLLADA_BUILDNAME}-1.4-d
    $ENV{COLLADA_DIR}/lib
    $ENV{COLLADA_DIR}/lib-dbg
    $ENV{COLLADA_DIR}
    ~/Library/Frameworks
    /Library/Frameworks
    /opt/local/Library/Frameworks #macports
    /usr/local/lib
    /usr/local/lib64
    /usr/lib
    /usr/lib64
    /sw/lib
    /opt/local/lib
    /opt/csw/lib
    /opt/lib
    /usr/freeware/lib64
    ${ACTUAL_3DPARTY_DIR}/lib
)

    # find extra libraries that the static linking requires

    FIND_PACKAGE(LibXml2)
    IF (LIBXML2_FOUND)
        SET(COLLADA_LIBXML_LIBRARY "${LIBXML2_LIBRARIES}" CACHE FILEPATH "" FORCE)
    ELSE(LIBXML2_FOUND)
        IF(WIN32)
            FIND_LIBRARY(COLLADA_LIBXML_LIBRARY
                NAMES libxml2
                PATHS
                ${COLLADA_DOM_ROOT}/external-libs/libxml2/win32/lib
                ${COLLADA_DOM_ROOT}/external-libs/libxml2/mingw/lib
                ${ACTUAL_3DPARTY_DIR}/lib
            )
        ENDIF(WIN32)
    ENDIF(LIBXML2_FOUND)
    
    FIND_PACKAGE(ZLIB)
    IF (ZLIB_FOUND)
        SET(COLLADA_ZLIB_LIBRARY "${ZLIB_LIBRARY}" CACHE FILEPATH "" FORCE)
    ELSE(ZLIB_FOUND)
        IF(WIN32)
            FIND_LIBRARY(COLLADA_ZLIB_LIBRARY
                NAMES zlib
                PATHS
                ${COLLADA_DOM_ROOT}/external-libs/libxml2/win32/lib
                ${COLLADA_DOM_ROOT}/external-libs/libxml2/mingw/lib
                ${ACTUAL_3DPARTY_DIR}/lib
            )
        ENDIF(WIN32)
    ENDIF(ZLIB_FOUND)

    FIND_LIBRARY(COLLADA_PCRECPP_LIBRARY
        NAMES pcrecpp
        PATHS
        ${COLLADA_DOM_ROOT}/external-libs/pcre/lib/${COLLADA_BUILDNAME}
        ${COLLADA_DOM_ROOT}/external-libs/pcre/lib/mac
        ${COLLADA_DOM_ROOT}/external-libs/pcre/lib/mingw
        ${ACTUAL_3DPARTY_DIR}/lib
    )

    FIND_LIBRARY(COLLADA_PCRECPP_LIBRARY_DEBUG
        NAMES pcrecpp-d pcrecppd
        PATHS
        ${COLLADA_DOM_ROOT}/external-libs/pcre/lib/${COLLADA_BUILDNAME}
        ${COLLADA_DOM_ROOT}/external-libs/pcre/lib/mac
        ${COLLADA_DOM_ROOT}/external-libs/pcre/lib/mingw
        ${ACTUAL_3DPARTY_DIR}/lib
    )

    FIND_LIBRARY(COLLADA_PCRE_LIBRARY
        NAMES pcre
        PATHS
        ${COLLADA_DOM_ROOT}/external-libs/pcre/lib/${COLLADA_BUILDNAME}
        ${COLLADA_DOM_ROOT}/external-libs/pcre/lib/mac
        ${COLLADA_DOM_ROOT}/external-libs/pcre/lib/mingw
        ${ACTUAL_3DPARTY_DIR}/lib
    )

    FIND_LIBRARY(COLLADA_PCRE_LIBRARY_DEBUG
        NAMES pcre-d pcred
        PATHS
        ${COLLADA_DOM_ROOT}/external-libs/pcre/lib/${COLLADA_BUILDNAME}
        ${COLLADA_DOM_ROOT}/external-libs/pcre/lib/mac
        ${COLLADA_DOM_ROOT}/external-libs/pcre/lib/mingw
        ${ACTUAL_3DPARTY_DIR}/lib
    )

    FIND_LIBRARY(COLLADA_MINIZIP_LIBRARY
        NAMES minizip
        PATHS
        ${COLLADA_DOM_ROOT}/external-libs/minizip/win32/lib
        ${COLLADA_DOM_ROOT}/external-libs/minizip/mac
        ${ACTUAL_3DPARTY_DIR}/lib
    )

    FIND_LIBRARY(COLLADA_MINIZIP_LIBRARY_DEBUG
        NAMES minizip-d minizipD
        PATHS
        ${COLLADA_DOM_ROOT}/external-libs/minizip/win32/lib
        ${COLLADA_DOM_ROOT}/external-libs/minizip/mac
        ${ACTUAL_3DPARTY_DIR}/lib
    )

    FIND_LIBRARY(COLLADA_BOOST_FILESYSTEM_LIBRARY
        NAMES libboost_filesystem boost_filesystem boost_filesystem-mt libboost_filesystem-${COLLADA_BUILDNAME}0-mt libboost_filesystem-${COLLADA_BUILDNAME}0-mt-1_54 libboost_filesystem-${COLLADA_BUILDNAME}0-mt-1_55 
        PATHS
        ${COLLADA_DOM_ROOT}/external-libs/boost/lib/${COLLADA_BUILDNAME}
        ${COLLADA_DOM_ROOT}/external-libs/boost/lib/mingw
        ${ACTUAL_3DPARTY_DIR}/lib
    )

    FIND_LIBRARY(COLLADA_BOOST_FILESYSTEM_LIBRARY_DEBUG
        NAMES libboost_filesystem-d boost_filesystem-d boost_filesystem-mt-d libboost_filesystem-${COLLADA_BUILDNAME}0-mt-gd libboost_filesystem-${COLLADA_BUILDNAME}0-mt-gd-1_54 libboost_filesystem-${COLLADA_BUILDNAME}0-mt-gd-1_55 
        PATHS
        ${COLLADA_DOM_ROOT}/external-libs/boost/lib/${COLLADA_BUILDNAME}
        ${COLLADA_DOM_ROOT}/external-libs/boost/lib/mingw
        ${ACTUAL_3DPARTY_DIR}/lib
    )

    FIND_LIBRARY(COLLADA_BOOST_SYSTEM_LIBRARY
        NAMES libboost_system boost_system boost_system-mt libboost_system-${COLLADA_BUILDNAME}0-mt libboost_system-${COLLADA_BUILDNAME}0-mt-1_54 libboost_system-${COLLADA_BUILDNAME}0-mt-1_55 
        PATHS
        ${COLLADA_DOM_ROOT}/external-libs/boost/lib/${COLLADA_BUILDNAME}
        ${COLLADA_DOM_ROOT}/external-libs/boost/lib/mingw
        ${ACTUAL_3DPARTY_DIR}/lib
    )

    FIND_LIBRARY(COLLADA_BOOST_SYSTEM_LIBRARY_DEBUG
        NAMES libboost_system-d boost_system-d boost_system-mt-d libboost_system-${COLLADA_BUILDNAME}0-mt-gd libboost_system-${COLLADA_BUILDNAME}0-mt-gd-1_54 libboost_system-${COLLADA_BUILDNAME}0-mt-gd-1_55 
        PATHS
        ${COLLADA_DOM_ROOT}/external-libs/boost/lib/${COLLADA_BUILDNAME}
        ${COLLADA_DOM_ROOT}/external-libs/boost/lib/mingw
        ${ACTUAL_3DPARTY_DIR}/lib
    )


SET(COLLADA_FOUND "NO")
IF(COLLADA_DYNAMIC_LIBRARY OR COLLADA_STATIC_LIBRARY)
    IF   (COLLADA_INCLUDE_DIR)
        SET(COLLADA_FOUND "YES")
    ENDIF(COLLADA_INCLUDE_DIR)
ENDIF(COLLADA_DYNAMIC_LIBRARY OR COLLADA_STATIC_LIBRARY)
