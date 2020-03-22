#---
# File: FindLASlib.cmake
#
# Find the native libLASlib includes and library
#
#  LASlib_INCLUDE_DIRS - where to find libLASlib include files.
#  LASlib_LIBRARIES    - List of libraries when using libLASlib.
#  LASlib_FOUND        - True if libLASlib found.
#---


# Set the include dir:
find_path(LASlib_INCLUDE_DIR LASlib/lasreader_las.hpp)

# Macro for setting libraries:
macro(FIND_LASlib_LIBRARY MYLIBRARY MYLIBRARYNAME)
    find_library(
        ${MYLIBRARY}_DEBUG
        NAMES
            "${MYLIBRARYNAME}${CMAKE_DEBUG_POSTFIX}"
        PATHS
            ${LASlib_ROOT}
            $ENV{LASlib_ROOT}
            /usr/local
            /usr
            /opt/local
            /opt
        PATH_SUFFIXES
            Debug/lib/LASlib
            Debug/lib64/LASlib
            lib/LASlib
            lib64/LASlib
    )

    find_library(
        ${MYLIBRARY}
        NAMES
            "${MYLIBRARYNAME}${CMAKE_RELEASE_POSTFIX}"
        PATHS
            ${LASlib_ROOT}
            $ENV{LASlib_ROOT}
            /usr/local
            /usr
            /opt/local
            /opt
        PATH_SUFFIXES
            Release/lib/LASlib
            Release/lib64/LASlib
            lib/LASlib
            lib64/LASlib
    )
endmacro()

FIND_LASlib_LIBRARY(LASlib_LIBRARY LASlib)

set(LASlib_FOUND "NO")
if (LIBLAS_LIBRARY AND LIBLAS_INCLUDE_DIR)
    set(LASlib_FOUND "YES")
endif()
