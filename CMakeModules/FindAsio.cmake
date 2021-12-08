# Locate ASIO-headers (http://think-async.com/Asio)
# This module defines
# ASIO_FOUND, if false, do not try to link to gdal
# ASIO_INCLUDE_DIR, where to find the headers
#
# Created by Stephan Maximilian Huber

FIND_PATH(ASIO_INCLUDE_DIR
  NAMES
    asio.hpp
  PATHS
    /usr/include
    /usr/local/include
)

SET(ASIO_FOUND "NO")
IF(ASIO_INCLUDE_DIR)

    set(ASIO_VERSION_H ${ASIO_INCLUDE_DIR}/asio/version.hpp)
    file(STRINGS  ${ASIO_VERSION_H} AsioVersionLine REGEX "^#define ASIO_VERSION ")
    string(REGEX MATCHALL "[0-9]+" AsioHeaderVersionMatches "${AsioVersionLine}")
    list(GET AsioHeaderVersionMatches 0 AsioHeaderVersion)

    # check version is less than 1.14.0 otherwise API changes break build
    if (${AsioHeaderVersion} LESS "101400")
        FIND_PACKAGE( Boost 1.37 )
        IF(Boost_FOUND)
            SET(ASIO_FOUND "YES")
        ENDIF()
    else()
        message("ASIO not compatible")
    endif()

ENDIF()
