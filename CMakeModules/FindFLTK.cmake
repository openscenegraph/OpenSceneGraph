# Locate FLTK
# This module defines
# FLTK_LIBRARY
# FLTK_FOUND, if false, do not try to link to gdal 
# FLTK_INCLUDE_DIR, where to find the headers
#
# $FLTK_DIR is an environment variable that would
# correspond to the ./configure --prefix=$FLTK_DIR
#
# Created by Robert Osfield. 

# prefer FindFLTK from cmake distribution
if(EXISTS ${CMAKE_ROOT}/Modules/FindFLTK.cmake)
  include(${CMAKE_ROOT}/Modules/FindFLTK.cmake)

  if(FLTK_FOUND)
    return()
  endif()
endif()

FIND_PATH(FLTK_INCLUDE_DIR Fl/Fl.H Fl/Fl.h
    $ENV{FLTK_DIR}/include
    $ENV{FLTK_DIR}
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

MACRO(FIND_FLTK_LIBRARY MYLIBRARY MYLIBRARYNAME)

    FIND_LIBRARY(${MYLIBRARY}
        NAMES ${MYLIBRARYNAME}
        PATHS
        $ENV{FLTK_DIR}/lib
        $ENV{FLTK_DIR}
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

ENDMACRO(FIND_FLTK_LIBRARY LIBRARY LIBRARYNAME)

FIND_FLTK_LIBRARY(FLTK_LIBRARY fltk)
FIND_FLTK_LIBRARY(FLTK_GL_LIBRARY fltk_gl)

SET(FLTK_FOUND "NO")
IF(FLTK_LIBRARY AND FLTK_INCLUDE_DIR)
    SET(FLTK_FOUND "YES")
ENDIF(FLTK_LIBRARY AND FLTK_INCLUDE_DIR)
