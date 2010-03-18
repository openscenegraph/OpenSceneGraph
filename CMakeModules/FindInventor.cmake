#
# Find Open Inventor
#
# This module defines:
#  INVENTOR_FOUND, if false, do not try to link against Inventor.
#  INVENTOR_INCLUDE_DIR, where to find headers.
#  INVENTOR_LIBRARY, the library to link against.
#  INVENTOR_LIBRARY_DEBUG, the debug library to link against.
#  INVENTOR_SOWIN_LIBRARY, the SoWin library - window binding library for Inventor
#  INVENTOR_SOWIN_LIBRARY, the SoWin debug library
#  INVENTOR_SOXT_LIBRARY, the SoXt library - window binding library for Inventor
#  INVENTOR_SOXT_LIBRARY, the SoXt debug library
#


#
#  Inventor
#
#  notes:
#  - Coin is honored over SGI Inventor
#  - Coin is detected by coin-config script, COINDIR environment variable,
#    and finally standard system locations are searched
#  - SGI Inventor is searched at standard system locations only
#

# coin-config tells much of Coin instalation (if present)
execute_process (COMMAND coin-config --prefix
                 OUTPUT_VARIABLE COIN_PREFIX
                 OUTPUT_STRIP_TRAILING_WHITESPACE)

# try to find Inventor includes (priority paths)
FIND_PATH(INVENTOR_INCLUDE_DIR Inventor/So.h
    ${COIN_PREFIX}/include
    $ENV{COINDIR}/include
    NO_DEFAULT_PATH
)

# try to find Inventor includes (regular paths)
FIND_PATH(INVENTOR_INCLUDE_DIR Inventor/So.h
    /usr/local/include
    /usr/include
    /sw/include
    /opt/local/include
    /opt/csw/include
    /opt/include
)

# default Inventor lib search paths
SET(INVENTOR_LIB_SEARCH_PATH
    /usr/local/lib
    /usr/lib
    /sw/lib
    /opt/local/lib
    /opt/csw/lib
    /opt/lib
)

# try to find Coin release lib (priority paths)
FIND_LIBRARY(INVENTOR_LIBRARY_RELEASE
    NAMES coin5 coin4 coin3 coin2 coin1 Coin
    PATHS ${COIN_PREFIX}/lib
          $ENV{COINDIR}/lib
    NO_DEFAULT_PATH
)

# try to find Coin release lib (regular paths)
FIND_LIBRARY(INVENTOR_LIBRARY_RELEASE
    NAMES coin5 coin4 coin3 coin2 coin1 Coin
    PATHS ${INVENTOR_LIB_SEARCH_PATH}
)

# try to find SGI Inventor lib
FIND_LIBRARY(INVENTOR_LIBRARY_RELEASE
    NAMES Inventor
    PATHS ${INVENTOR_LIB_SEARCH_PATH}
)

# try to find Coin debug lib (priority paths)
FIND_LIBRARY(INVENTOR_LIBRARY_DEBUG
    NAMES coin5d coin4d coin3d coin2d coin1d
    PATHS ${COIN_PREFIX}/lib
          $ENV{COINDIR}/lib
    NO_DEFAULT_PATH
)

# try to find Coin debug lib (regular paths)
FIND_LIBRARY(INVENTOR_LIBRARY_DEBUG
    NAMES coin5d coin4d coin3d coin2d coin1d
    PATHS ${INVENTOR_LIB_SEARCH_PATH}
)

# set release to debug if only debug found
IF(NOT INVENTOR_LIBRARY_RELEASE AND INVENTOR_LIBRARY_DEBUG)
  SET(INVENTOR_LIBRARY_RELEASE ${INVENTOR_LIBRARY_DEBUG})
ENDIF(NOT INVENTOR_LIBRARY_RELEASE AND INVENTOR_LIBRARY_DEBUG)

# set debug to release (if only release found)
IF(NOT INVENTOR_LIBRARY_DEBUG AND INVENTOR_LIBRARY_RELEASE)
  SET(INVENTOR_LIBRARY_DEBUG ${INVENTOR_LIBRARY_RELEASE})
ENDIF(NOT INVENTOR_LIBRARY_DEBUG AND INVENTOR_LIBRARY_RELEASE)

# INVENTOR_LIBRARY
IF (CMAKE_CONFIGURATION_TYPES OR CMAKE_BUILD_TYPE)
  SET(INVENTOR_LIBRARY
      optimized ${INVENTOR_LIBRARY_RELEASE}
      debug ${INVENTOR_LIBRARY_DEBUG})
ELSE(CMAKE_CONFIGURATION_TYPES OR CMAKE_BUILD_TYPE)
  SET(INVENTOR_LIBRARY
      ${INVENTOR_LIBRARY_RELEASE})
ENDIF(CMAKE_CONFIGURATION_TYPES OR CMAKE_BUILD_TYPE)

# set INVENTOR_FOUND appropriately
SET(INVENTOR_FOUND "NO")
IF(INVENTOR_INCLUDE_DIR AND INVENTOR_LIBRARY)
    SET(INVENTOR_FOUND "YES")
ENDIF(INVENTOR_INCLUDE_DIR AND INVENTOR_LIBRARY)


#
#  SoWin
#
# notes: SoWin is searched by COINDIR environment variable
#        (as expected to be located at Windows platform)

# try to find SoWin lib (priority paths)
FIND_LIBRARY(INVENTOR_SOWIN_LIBRARY
    NAMES sowin1
    PATHS $ENV{COINDIR}/lib
    NO_DEFAULT_PATH
)

# try to find SoWin lib (regular paths)
FIND_LIBRARY(INVENTOR_SOWIN_LIBRARY
    NAMES sowin1
    PATHS ${INVENTOR_LIB_SEARCH_PATH}
)

# try to find SoWin debug lib (priority paths)
FIND_LIBRARY(INVENTOR_SOWIN_LIBRARY_DEBUG
    NAMES sowin1d
    PATHS $ENV{COINDIR}/lib
    NO_DEFAULT_PATH
)

# try to find SoWin debug lib (regular paths)
FIND_LIBRARY(INVENTOR_SOWIN_LIBRARY_DEBUG
    NAMES sowin1d
    PATHS ${INVENTOR_LIB_SEARCH_PATH}
)

# SoWin debug library defaults to non-debug lib
IF(NOT INVENTOR_SOWIN_LIBRARY_DEBUG)
    IF(INVENTOR_SOWIN_LIBRARY)
        SET(INVENTOR_SOWIN_LIBRARY_DEBUG INVENTOR_SOWIN_LIBRARY)
    ENDIF(INVENTOR_SOWIN_LIBRARY)
ENDIF(NOT INVENTOR_SOWIN_LIBRARY_DEBUG)


#
#  SoXt
#
#  notes:
#  - SoXt is detected by soxt-config script (as expected by
#    Coin's SoXt on Linux/Unix) and on standard system locations
#  - SGI's InventorXt support is missing now
#

# soxt-config tells much of SoXt instalation (if present)
execute_process (COMMAND soxt-config --prefix
                 OUTPUT_VARIABLE SOXT_PREFIX
                 OUTPUT_STRIP_TRAILING_WHITESPACE)

# try to find SoXt lib (priority paths)
FIND_LIBRARY(INVENTOR_SOXT_LIBRARY
    NAMES soxt1 SoXt
    PATHS ${SOXT_PREFIX}/lib
          $ENV{COINDIR}/lib
    NO_DEFAULT_PATH
)

# try to find SoXt lib (regular paths)
FIND_LIBRARY(INVENTOR_SOXT_LIBRARY
    NAMES soxt1 SoXt
    PATHS ${INVENTOR_LIB_SEARCH_PATH}
)

# try to find SoXt debug lib (priority paths)
FIND_LIBRARY(INVENTOR_SOXT_LIBRARY_DEBUG
    NAMES soxt1d
    PATHS ${SOXT_PREFIX}/lib
          $ENV{COINDIR}/lib
    NO_DEFAULT_PATH
)

# try to find SoXt debug lib (regular paths)
FIND_LIBRARY(INVENTOR_SOXT_LIBRARY_DEBUG
    NAMES soxt1d
    PATHS ${INVENTOR_LIB_SEARCH_PATH}
)

# SoXt debug library defaults to non-debug lib
IF(NOT INVENTOR_SOXT_LIBRARY_DEBUG)
    IF(INVENTOR_SOXT_LIBRARY)
        SET(INVENTOR_SOXT_LIBRARY_DEBUG INVENTOR_SOXT_LIBRARY)
    ENDIF(INVENTOR_SOXT_LIBRARY)
ENDIF(NOT INVENTOR_SOXT_LIBRARY_DEBUG)
