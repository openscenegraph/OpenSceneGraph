INCLUDE_DIRECTORIES( ${COLLADA_INCLUDE_DIR} ${COLLADA_INCLUDE_DIR}/1.4)

SET(TARGET_SRC
    daeReader.cpp
    daeRGeometry.cpp
    daeRMaterials.cpp
    daeRSceneObjects.cpp
    daeRTransforms.cpp
    daeWGeometry.cpp
    daeWMaterials.cpp
    daeWriter.cpp
    daeWSceneObjects.cpp
    daeWTransforms.cpp
    domSourceReader.cpp
    ReaderWriterDAE.cpp
)
SET(TARGET_H 
    daeReader.h
    daeWriter.h
    domSourceReader.h
)
SET(TARGET_EXTERNAL_LIBRARIES collada_dom collada_dae collada_STLDatabase collada_LIBXMLPlugin collada_stdErrPlugin xml2 )
SET(TARGET_LIBRARIES_VARS COLLADA_LIBRARY)

#### end var setup  ###
SETUP_PLUGIN(dae)
