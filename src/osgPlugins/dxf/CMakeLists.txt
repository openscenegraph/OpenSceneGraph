SET(TARGET_SRC
    ReaderWriterDXF.cpp
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
)

SET(TARGET_ADDED_LIBRARIES osgText )

#### end var setup  ###
SETUP_PLUGIN(dxf)
