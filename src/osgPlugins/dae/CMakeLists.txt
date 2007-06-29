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
IF(WIN32)
    SET(TARGET_EXTERNAL_LIBRARIES libcollada_dom libcollada_dae libcollada_STLDatabase libcollada_LIBXMLPlugin libcollada_stdErrPlugin libxml2 )
ELSE(WIN32)
    #SET(TARGET_EXTERNAL_LIBRARIES collada_dom collada_dae collada_STLDatabase collada_LIBXMLPlugin collada_stdErrPlugin xml2 )
    # JC hack STLDatabase and XMLPlugin are statically linked on Linux
    SET(TARGET_EXTERNAL_LIBRARIES collada_dom collada_dae xml2 )
ENDIF(WIN32)
SET(TARGET_LIBRARIES_VARS COLLADA_LIBRARY)

#### end var setup  ###
SETUP_PLUGIN(dae)
