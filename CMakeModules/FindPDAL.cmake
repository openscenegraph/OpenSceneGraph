#---
# File: FindPDAL.cmake
#
# Find the native PDAL includes and library
#
#  PDAL_INCLUDE_DIRS - where to find pdal's includes.
#  PDAL_LIBRARIES    - List of libraries when using pdal.
#  PDAL_FOUND        - True if pdal found.
#---


# Set the include dir:
find_path(PDAL_INCLUDE_DIR pdal/pdal.hpp)
find_path(LASZIP_INCLUDE_DIR laszip/laszip_api.h)

# Macro for setting libraries:
macro(FIND_PDAL_LIBRARY MYLIBRARY MYLIBRARYNAME)

   find_library(
     "${MYLIBRARY}_DEBUG"
     NAMES "${MYLIBRARYNAME}${CMAKE_DEBUG_POSTFIX}" "lib${MYLIBRARYNAME}${CMAKE_DEBUG_POSTFIX}"
     PATHS
     ${PDAL_DIR}/lib/Debug
     ${PDAL_DIR}/lib64/Debug
     ${PDAL_DIR}/lib
     ${PDAL_DIR}/lib64
     $ENV{PDAL_DIR}/lib/debug
     $ENV{PDAL_DIR}/lib64/debug
     NO_DEFAULT_PATH
   )

   find_library(
     "${MYLIBRARY}_DEBUG"
     NAMES "${MYLIBRARYNAME}${CMAKE_DEBUG_POSTFIX}" "lib${MYLIBRARYNAME}${CMAKE_DEBUG_POSTFIX}"
     PATHS
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
     [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;PDAL_ROOT]/lib
     /usr/freeware/lib64
   )

   find_library(
     ${MYLIBRARY}
     NAMES "${MYLIBRARYNAME}${CMAKE_RELEASE_POSTFIX}" "lib${MYLIBRARYNAME}${CMAKE_RELEASE_POSTFIX}"
     PATHS
     ${PDAL_DIR}/lib/Release
     ${PDAL_DIR}/lib64/Release
     ${PDAL_DIR}/lib
     ${PDAL_DIR}/lib64
     $ENV{PDAL_DIR}/lib/Release
     $ENV{PDAL_DIR}/lib64/Release
     $ENV{PDAL_DIR}/lib
     $ENV{PDAL_DIR}/lib64
     $ENV{PDAL_DIR}
     $ENV{PDALDIR}/lib
     $ENV{PDALDIR}/lib64
     $ENV{PDALDIR}
     $ENV{PDAL_ROOT}/lib
     $ENV{PDAL_ROOT}/lib64
     NO_DEFAULT_PATH
   )

   find_library(
     ${MYLIBRARY}
     NAMES "${MYLIBRARYNAME}${CMAKE_RELEASE_POSTFIX}" "lib${MYLIBRARYNAME}${CMAKE_RELEASE_POSTFIX}"
     PATHS
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
     [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;PDAL_ROOT]/lib
     /usr/freeware/lib64
   )

   if( NOT ${MYLIBRARY}_DEBUG )
     if( MYLIBRARY )
       set( ${MYLIBRARY}_DEBUG ${MYLIBRARY} )
     endif(MYLIBRARY)
   endif( NOT ${MYLIBRARY}_DEBUG )

endmacro(FIND_PDAL_LIBRARY LIBRARY LIBRARYNAME)

FIND_PDAL_LIBRARY(PDALCPP_LIBRARY pdalcpp)
set(PDAL_FOUND "NO")
if(WIN32)
    FIND_PDAL_LIBRARY(PDALUTIL_LIBRARY pdal_util)
    if(PDALCPP_LIBRARY AND PDALUTIL_LIBRARY AND PDAL_INCLUDE_DIR)
        set(PDAL_INCLUDE_DIRS ${PDAL_INCLUDE_DIR} ${LASZIP_INCLUDE_DIR})
        set(PDAL_LIBRARIES ${PDALCPP_LIBRARY} ${PDALUTIL_LIBRARY} )
        set(PDAL_FOUND "YES")
    endif()
else(WIN32)
    if(PDALCPP_LIBRARY AND PDAL_INCLUDE_DIR)
        set(PDAL_INCLUDE_DIRS ${PDAL_INCLUDE_DIR} ${LASZIP_INCLUDE_DIR})
        set(PDAL_LIBRARIES ${PDALCPP_LIBRARY} )
        set(PDAL_FOUND "YES")
    endif()
endif(WIN32)
