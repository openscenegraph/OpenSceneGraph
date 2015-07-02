INCLUDE_DIRECTORIES( ${GDAL_INCLUDE_DIR} )

SET(TARGET_SRC
    ReaderWriterGDAL.cpp
    DataSetLayer.cpp
)

SET(TARGET_H
    DataSetLayer.h
)

SET(TARGET_LIBRARIES_VARS GDAL_LIBRARY )
SET(TARGET_ADDED_LIBRARIES osgTerrain )

#### end var setup  ###
SETUP_PLUGIN(gdal)
