# Locate libgta
# This module defines
# GTA_FOUND, if false, do not try to link to libgta
# GTA_INCLUDE_DIRS, where to find the headers
# GTA_LIBRARIES
#
# $GTA_DIR is an environment variable that would
# correspond to the ./configure --prefix=$GTA_DIR
# used in building libgta.

INCLUDE(FindPkgConfig OPTIONAL)

IF(PKG_CONFIG_FOUND)

    INCLUDE(FindPkgConfig)

    PKG_CHECK_MODULES(GTA gta)

ELSE(PKG_CONFIG_FOUND)

FIND_PATH(GTA_INCLUDE_DIRS gta/gta.hpp
    $ENV{GTA_DIR}/include
    $ENV{GTA_DIR}
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

FIND_LIBRARY(GTA_LIBRARIES 
    NAMES gta libgta
    PATHS
    $ENV{GTA_DIR}/lib
    $ENV{GTA_DIR}
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

SET(GTA_FOUND "NO")
IF(GTA_LIBRARIES AND GTA_INCLUDE_DIRS)
    SET(GTA_FOUND "YES")
ENDIF(GTA_LIBRARIES AND GTA_INCLUDE_DIRS)

ENDIF(PKG_CONFIG_FOUND)
