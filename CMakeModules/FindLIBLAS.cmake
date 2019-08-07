#---
# File: FindLIBLAS.cmake
#
# Find the native LIBLAS includes and library
#
#  LIBLAS_INCLUDE_DIRS - where to find liblas's includes.
#  LIBLAS_LIBRARIES    - List of libraries when using liblas.
#  LIBLAS_FOUND        - True if liblas found.
#---


# Set the include dir:
find_path(LIBLAS_INCLUDE_DIR liblas/liblas.hpp)

# Macro for setting libraries:
macro(FIND_LIBLAS_LIBRARY MYLIBRARY MYLIBRARYNAME)

   find_library(
     "${MYLIBRARY}_DEBUG"
     NAMES "${MYLIBRARYNAME}${CMAKE_DEBUG_POSTFIX}" "lib${MYLIBRARYNAME}${CMAKE_DEBUG_POSTFIX}"
     PATHS
     ${LIBLAS_DIR}/lib/Debug
     ${LIBLAS_DIR}/lib64/Debug
     ${LIBLAS_DIR}/lib
     ${LIBLAS_DIR}/lib64
     $ENV{LIBLAS_DIR}/lib/debug
     $ENV{LIBLAS_DIR}/lib64/debug
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
     [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;LIBLAS_ROOT]/lib
     /usr/freeware/lib64
   )

   find_library(
     ${MYLIBRARY}
     NAMES "${MYLIBRARYNAME}${CMAKE_RELEASE_POSTFIX}" "lib${MYLIBRARYNAME}${CMAKE_RELEASE_POSTFIX}"
     PATHS
     ${LIBLAS_DIR}/lib/Release
     ${LIBLAS_DIR}/lib64/Release
     ${LIBLAS_DIR}/lib
     ${LIBLAS_DIR}/lib64
     $ENV{LIBLAS_DIR}/lib/Release
     $ENV{LIBLAS_DIR}/lib64/Release
     $ENV{LIBLAS_DIR}/lib
     $ENV{LIBLAS_DIR}/lib64
     $ENV{LIBLAS_DIR}
     $ENV{LIBLASDIR}/lib
     $ENV{LIBLASDIR}/lib64
     $ENV{LIBLASDIR}
     $ENV{LIBLAS_ROOT}/lib
     $ENV{LIBLAS_ROOT}/lib64
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
     [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;LIBLAS_ROOT]/lib
     /usr/freeware/lib64
   )

   if( NOT ${MYLIBRARY}_DEBUG )
     if( MYLIBRARY )
       set( ${MYLIBRARY}_DEBUG ${MYLIBRARY} )
     endif(MYLIBRARY)
   endif( NOT ${MYLIBRARY}_DEBUG )

endmacro(FIND_LIBLAS_LIBRARY LIBRARY LIBRARYNAME)

FIND_LIBLAS_LIBRARY(LIBLAS_LIBRARY las)

set(LIBLAS_FOUND "NO")
if(LIBLAS_LIBRARY AND LIBLAS_INCLUDE_DIR)
    FIND_PACKAGE(Boost) # used by LIBLAS
    if(Boost_FOUND)
        set(LIBLAS_LIBRARIES ${LIBLAS_LIBRARY} )
        set(LIBLAS_FOUND "YES")
        if(WIN32)
            link_directories(${Boost_LIBRARY_DIRS})
        endif()
    endif()
endif()
