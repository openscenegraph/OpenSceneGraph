SET(TARGET_SRC
    ReaderWriterDXF.cpp
    DXFWriterNodeVisitor.cpp
    aci.cpp
    dxfBlock.cpp
    dxfEntity.cpp
    dxfFile.cpp
    dxfReader.cpp
    dxfSection.cpp
    dxfTable.cpp
    scene.cpp
)

SET(TARGET_H
    aci.h
    codeValue.h
    dxfBlock.h
    dxfDataTypes.h
    dxfEntity.h
    dxfFile.h
    dxfReader.h
    dxfSection.h
    dxfSectionBase.h
    dxfTable.h
    scene.h
    DXFWriterNodeVisitor.h
)

IF(GDAL_FOUND)
	INCLUDE_DIRECTORIES( ${GDAL_INCLUDE_DIR} )
	SET(TARGET_LIBRARIES_VARS GDAL_LIBRARY )
	ADD_DEFINITIONS(-DUSE_GDAL)
ENDIF(GDAL_FOUND)

SET(TARGET_ADDED_LIBRARIES osgText )


#### end var setup  ###
SETUP_PLUGIN(dxf)
