INCLUDE_DIRECTORIES( ${COLLADA_INCLUDE_DIR} )

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

SET(TARGET_LIBRARIES_VARS COLLADA_LIBRARY )

#### end var setup  ###
SETUP_PLUGIN(dae)
