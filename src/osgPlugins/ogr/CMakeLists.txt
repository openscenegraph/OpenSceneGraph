INCLUDE_DIRECTORIES( ${GDAL_INCLUDE_DIR} )

SET(TARGET_SRC ReaderWriterOGR.cpp)

SET(TARGET_LIBRARIES_VARS GDAL_LIBRARY )

IF(CMAKE_COMPILER_IS_GNUCXX)
    # Remove -Wshadow flag as it barfs on ffmoeg headers
   STRING(REGEX REPLACE "-Wshadow" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
ENDIF()

#### end var setup  ###
SETUP_PLUGIN(ogr)
