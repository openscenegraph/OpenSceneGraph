SET(TARGET_SRC
    ESRIShape.cpp
    ESRIShapeParser.cpp
    ESRIShapeReaderWriter.cpp
    XBaseParser.cpp)
SET(TARGET_H
    ESRIShape.h
    ESRIShapeParser.h
    XBaseParser.h)
SET(TARGET_ADDED_LIBRARIES
    osgSim
    osgTerrain)
#### end var setup  ###
SETUP_PLUGIN(shp)
