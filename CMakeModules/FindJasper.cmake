# Locate gdal
# This module defines
# JASPER_LIBRARY
# JASPER_FOUND, if false, do not try to link to gdal 
# JASPER_INCLUDE_DIR, where to find the headers
#
# $JASPER_DIR is an environment variable that would
# correspond to the ./configure --prefix=$JASPER_DIR
#
# Created by Robert Osfield. 

# prefer FindJasper from cmake distribution
if(EXISTS ${CMAKE_ROOT}/Modules/FindJasper.cmake)
  include(${CMAKE_ROOT}/Modules/FindJasper.cmake)

  if(JASPER_FOUND)
    return()
  endif()
endif()

FIND_PATH(JASPER_INCLUDE_DIR jasper/jasper.h
    $ENV{JASPER_DIR}/include
    $ENV{JASPER_DIR}/src/libjasper/include
    $ENV{JASPER_DIR}
    ~/Library/Frameworks
    /Library/Frameworks
    /usr/local/include
    /usr/include
    /sw/include # Fink
    /opt/local/include # DarwinPorts
    /opt/csw/include # Blastwave
    /opt/include
    /usr/freeware/include
)

FIND_LIBRARY(JASPER_LIBRARY 
    NAMES jasper libjasper
    PATHS
    $ENV{JASPER_DIR}/lib
    $ENV{JASPER_DIR}/src/libjasper/lib
    $ENV{JASPER_DIR}/src/msvc/Win32_Release
    $ENV{JASPER_DIR}
    ~/Library/Frameworks
    /Library/Frameworks
    /usr/local/lib
    /usr/lib
    /sw/lib
    /opt/local/lib
    /opt/csw/lib
    /opt/lib
    /usr/freeware/lib64
)

FIND_LIBRARY(JASPER_LIBRARY_DEBUG
    NAMES jasper libjasper jasperd libjasperd
    PATHS
    $ENV{JASPER_DIR}/lib
    $ENV{JASPER_DIR}/src/libjasper/lib
    $ENV{JASPER_DIR}/src/msvc/Win32_Debug
    $ENV{JASPER_DIR}
    ~/Library/Frameworks
    /Library/Frameworks
    /usr/local/lib
    /usr/lib
    /sw/lib
    /opt/local/lib
    /opt/csw/lib
    /opt/lib
    /usr/freeware/lib64
)

SET(JASPER_FOUND "NO")
IF(JASPER_LIBRARY AND JASPER_INCLUDE_DIR)
    SET(JASPER_FOUND "YES")
ENDIF(JASPER_LIBRARY AND JASPER_INCLUDE_DIR)

