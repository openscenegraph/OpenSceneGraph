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
    SET(FBX_LIBNAME "fbxsdk_gcc4_ub")
ELSEIF(CMAKE_COMPILER_IS_GNUCXX)
    SET(FBX_LIBNAME "fbxsdk_gcc4")
    #TODO: libs are provided for GCC 3.4 & 4.0 in both 32 and 64 bit versions
    # but I don't know how to confgure that here.
ELSEIF(MSVC71)
    SET(FBX_LIBNAME "fbxsdk_md2003")
ELSEIF(MSVC80)
    SET(FBX_LIBNAME "fbxsdk_md2005")
ELSEIF(MSVC90)
    SET(FBX_LIBNAME "fbxsdk_md2008")
ELSEIF(MSVC100 OR MSVC_VER>1600)
    SET(FBX_LIBNAME "fbxsdk_md2010")
ENDIF()

IF(CMAKE_CL_64)
    SET(FBX_LIBNAME ${FBX_LIBNAME}_amd64)
ENDIF()

IF(APPLE)
    SET(FBX_LIBNAME_DEBUG ${FBX_LIBNAME})
ELSE()
    SET(FBX_LIBNAME_DEBUG ${FBX_LIBNAME}d)
ENDIF()

# SET final path
#osg_fbx code is compatible with 2011.2 and 2011.3 so find either directory
SET( FBX_SEARCH_PATHS 
    $ENV{FBX_DIR}
    $ENV{PROGRAMFILES}/Autodesk/FBX/FbxSdk/2011.3
    /Applications/Autodesk/FBXSDK20113
    $ENV{PROGRAMFILES}/Autodesk/FBX/FbxSdk/2011.2
    /Applications/Autodesk/FBXSDK20112
)

# search for headers & debug/release libraries
FIND_PATH(FBX_INCLUDE_DIR fbxsdk.h
    PATHS ${FBX_SEARCH_PATHS}
    PATH_SUFFIXES include )
FIND_LIBRARY( FBX_LIBRARY ${FBX_LIBNAME}
    PATHS ${FBX_SEARCH_PATHS}
    PATH_SUFFIXES lib)
FIND_LIBRARY( FBX_LIBRARY_DEBUG ${FBX_LIBNAME_DEBUG}
    PATHS ${FBX_SEARCH_PATHS}
    PATH_SUFFIXES lib)

IF(FBX_LIBRARY AND FBX_LIBRARY_DEBUG AND FBX_INCLUDE_DIR)
    SET(FBX_FOUND "YES")
ELSE()
    SET(FBX_FOUND "NO")
ENDIF()
