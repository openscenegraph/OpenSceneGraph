# Locate FBX
# This module defines:
# FBX_INCLUDE_DIR, where to find the headers
#
# FBX_LIBRARY, FBX_LIBRARY_DEBUG
# FBX_FOUND
#
# $FBX_DIR is an environment variable that would
# correspond to the ./configure --prefix=$FBX_DIR

IF(APPLE)
    SET(FBX_LIBDIR "gcc4/ub")
ELSEIF(CMAKE_COMPILER_IS_GNUCXX)
    SET(FBX_LIBDIR "gcc4")
ELSEIF(MSVC80)
    SET(FBX_LIBDIR "vs2005")
ELSEIF(MSVC90)
    SET(FBX_LIBDIR "vs2008")
ELSEIF(MSVC10)
    SET(FBX_LIBDIR "vs2010")
ELSEIF(MSVC11 OR MSVC_VERSION>1700)
    SET(FBX_LIBDIR "vs2012")
ENDIF()

IF(APPLE)
    # do nothing
ELSEIF(CMAKE_CL_64)
    SET(FBX_LIBDIR ${FBX_LIBDIR}/x64)
ELSEIF(CMAKE_COMPILER_IS_GNUCXX AND CMAKE_SIZEOF_VOID_P EQUAL 8)
    SET(FBX_LIBDIR ${FBX_LIBDIR}/x64)
ELSE()
    SET(FBX_LIBDIR ${FBX_LIBDIR}/x86)
ENDIF()

IF(APPLE)
    SET(FBX_LIBNAME "libfbxsdk")
ELSEIF(CMAKE_COMPILER_IS_GNUCXX)
    SET(FBX_LIBNAME "fbxsdk")
ELSE()
    SET(FBX_LIBNAME "libfbxsdk-md")
ENDIF()

SET(FBX_LIBNAME_DEBUG ${FBX_LIBNAME}d)

SET( FBX_SEARCH_PATHS
    $ENV{FBX_DIR}
    "$ENV{ProgramW6432}/Autodesk/FBX/FBX SDK/2014.1"
    "$ENV{PROGRAMFILES}/Autodesk/FBX/FBX SDK/2014.1"
    /Applications/Autodesk/FBXSDK20141
)

# search for headers & debug/release libraries
FIND_PATH(FBX_INCLUDE_DIR "fbxsdk.h"
    PATHS ${FBX_SEARCH_PATHS}
    PATH_SUFFIXES "include")
FIND_LIBRARY( FBX_LIBRARY ${FBX_LIBNAME}
    PATHS ${FBX_SEARCH_PATHS}
    PATH_SUFFIXES "lib/${FBX_LIBDIR}/release" "lib/${FBX_LIBDIR}")

#Once one of the calls succeeds the result variable will be set and stored in the cache so that no call will search again.

#no debug d suffix, search in debug folder only
FIND_LIBRARY( FBX_LIBRARY_DEBUG ${FBX_LIBNAME}
    PATHS ${FBX_SEARCH_PATHS}
    PATH_SUFFIXES "lib/${FBX_LIBDIR}/debug")
FIND_LIBRARY( FBX_LIBRARY_DEBUG ${FBX_LIBNAME_DEBUG}
    PATHS ${FBX_SEARCH_PATHS}
    PATH_SUFFIXES "lib/${FBX_LIBDIR}")

IF(FBX_LIBRARY AND FBX_LIBRARY_DEBUG AND FBX_INCLUDE_DIR)
    SET(FBX_FOUND "YES")
ELSE()
    SET(FBX_FOUND "NO")
ENDIF()

IF(NOT FBX_FOUND)
#try to use 2013.3 version
    IF(APPLE)
        SET(FBX_LIBNAME "fbxsdk-2013.3-static")
    ELSEIF(CMAKE_COMPILER_IS_GNUCXX)
        SET(FBX_LIBNAME "fbxsdk-2013.3-static")
    ELSE()
        SET(FBX_LIBNAME "fbxsdk-2013.3-md")
    ENDIF()

    SET(FBX_LIBNAME_DEBUG ${FBX_LIBNAME}d)

    SET( FBX_SEARCH_PATHS
        $ENV{FBX_DIR}
        $ENV{ProgramW6432}/Autodesk/FBX/FBX SDK/2013.3
        $ENV{PROGRAMFILES}/Autodesk/FBX/FBX SDK/2013.3
        /Applications/Autodesk/FBXSDK20141
    )

    # search for headers & debug/release libraries
    FIND_PATH(FBX_INCLUDE_DIR "fbxsdk.h"
        PATHS ${FBX_SEARCH_PATHS}
        PATH_SUFFIXES "include")
    FIND_LIBRARY( FBX_LIBRARY ${FBX_LIBNAME}
        PATHS ${FBX_SEARCH_PATHS}
        PATH_SUFFIXES "lib/${FBX_LIBDIR}")

    FIND_LIBRARY( FBX_LIBRARY_DEBUG ${FBX_LIBNAME_DEBUG}
        PATHS ${FBX_SEARCH_PATHS}
        PATH_SUFFIXES "lib/${FBX_LIBDIR}")
    IF(FBX_LIBRARY AND FBX_LIBRARY_DEBUG AND FBX_INCLUDE_DIR)
        SET(FBX_FOUND "YES")
    ELSE()
        SET(FBX_FOUND "NO")
    ENDIF()

ENDIF()
