# Locate ZeroConf / Bonjour
# This module defines
# ZEROCONF_LIBRARY
# ZEROCONF_FOUND, if false, do not try to link to gdal 
# ZEROCONF_INCLUDE_DIR, where to find the headers
#
# $ZEROCONF_DIR is an environment variable that would
# correspond to the ./configure --prefix=$ZEROCONF_DIR

# Created by Stephan Maximilian Huber 

SET(ZEROCONF_FOUND "NO")

IF(APPLE)
  # bonjour is part of the system on os x / ios
  SET(ZEROCONF_FOUND "YES")
ELSE()
  IF(WIN32)
    # find the Bonjour SDK
    FIND_PATH(ZEROCONF_INCLUDE_DIR dnssd.h
      $ENV{ZEROCONF_DIR}/include
      $ENV{ZEROCONF_DIR}
      NO_DEFAULT_PATH
    )
    FIND_PATH(ZEROCONF_INCLUDE_DIR dnssd.h
      PATHS ${CMAKE_PREFIX_PATH} # Unofficial: We are proposing this.
      NO_DEFAULT_PATH
      PATH_SUFFIXES include
    )
    FIND_PATH(ZEROCONF_INCLUDE_DIR dnssd.h)

    FIND_LIBRARY(ZEROCONF_LIBRARY dnssd
      PATHS ${CMAKE_PREFIX_PATH} # Unofficial: We are proposing this.
      NO_DEFAULT_PATH
      PATH_SUFFIXES lib64 lib
    )
    FIND_LIBRARY(ZEROCONF_LIBRARY dnssd)

    SET(ZEROCONF_FOUND "NO")
    IF(ZEROCONF_LIBRARY AND ZEROCONF_INCLUDE_DIR)
      SET(ZEROCONF_FOUND "YES")
    ENDIF()

  ELSE()
  	# TODO find AVAHI on linux
  ENDIF()
ENDIF()
