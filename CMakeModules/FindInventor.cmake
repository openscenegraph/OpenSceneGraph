# - Locate Inventor
# This module defines:
#  INVENTOR_FOUND, if false, do not try to link against Inventor.
#  INVENTOR_INCLUDE_DIR, where to find headers.
#  INVENTOR_LIBRARY, the library to link against.
#  INVENTOR_LIBRARY_DEBUG, the debug library to link against.

FIND_PATH(INVENTOR_INCLUDE_DIR Inventor/So.h
    /usr/local/include
    /usr/include
    /sw/include
    /opt/local/include
    /opt/csw/include
    /opt/include
    $ENV{COINDIR}/include
)

FIND_LIBRARY(INVENTOR_LIBRARY
    NAMES coin2 Coin
    PATHS
    /usr/local/lib
    /usr/lib
    /sw/lib
    /opt/local/lib
    /opt/csw/lib
    /opt/lib
    $ENV{COINDIR}/lib
)
IF(NOT INVENTOR_LIBRARY)
    # If we can't find libCoin try libInventor
    FIND_LIBRARY(INVENTOR_LIBRARY
        NAMES Inventor
        PATHS
        /usr/local/lib
        /usr/lib
        /sw/lib
        /opt/local/lib
        /opt/csw/lib
        /opt/lib
    )
ENDIF(NOT INVENTOR_LIBRARY)

FIND_LIBRARY(INVENTOR_LIBRARY_DEBUG
    NAMES coin2d
    PATHS
    /usr/local/lib
    /usr/lib
    /sw/lib
    /opt/local/lib
    /opt/csw/lib
    /opt/lib
    $ENV{COINDIR}/lib
)
IF(NOT INVENTOR_LIBRARY_DEBUG)
    IF(INVENTOR_LIBRARY)
        SET(INVENTOR_LIBRARY_DEBUG ${INVENTOR_LIBRARY})
    ENDIF(INVENTOR_LIBRARY)
ENDIF(NOT INVENTOR_LIBRARY_DEBUG)

SET(INVENTOR_FOUND "NO")
IF(INVENTOR_INCLUDE_DIR AND INVENTOR_LIBRARY)
    SET(INVENTOR_FOUND "YES")
ENDIF(INVENTOR_INCLUDE_DIR AND INVENTOR_LIBRARY)
