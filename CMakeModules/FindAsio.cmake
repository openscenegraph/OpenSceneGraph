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
FIND_PACKAGE( Boost 1.37 )
IF(Boost_FOUND AND ASIO_INCLUDE_DIR) 
  SET(ASIO_FOUND "YES")
ENDIF()
