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

OPTION(COLLADA_USE_STATIC "Set to ON to build OpenSceneGraph with static Collada support." OFF)
#MARK_AS_ADVANCED(COLLADA_USE_STATIC)

IF    (WIN32)

  IF   (COLLADA_USE_STATIC)
    SET(TARGET_EXTERNAL_LIBRARIES libxml2 pcrecpp-d pcre-d)
  ELSE (COLLADA_USE_STATIC)
#    ADD_DEFINITIONS(-DDOM_DYNAMIC)
    SET(TARGET_EXTERNAL_LIBRARIES libxml2 pcrecpp-d pcre-d)
  ENDIF(COLLADA_USE_STATIC)

ELSE (WIN32)

    # SET(TARGET_EXTERNAL_LIBRARIES collada_dom collada_dae collada_STLDatabase collada_LIBXMLPlugin collada_stdErrPlugin xml2 )
    # JC hack STLDatabase and XMLPlugin are statically linked on Linux
    #
    # This works in "shared" mode
    # SET(TARGET_EXTERNAL_LIBRARIES collada_dom_shared collada_dae_shared xml2 )
    #
    # This works in "static" mode
    # SET(TARGET_EXTERNAL_LIBRARIES collada_dom collada_dae xml2 pcrecpp)
    #
    # ...so what we need is a conditional way to properly SET() the right one.
    IF(COLLADA_USE_STATIC)
        SET(TARGET_EXTERNAL_LIBRARIES collada14dom xml2 pcrecpp)
    ELSE(COLLADA_USE_STATIC)
        SET(TARGET_EXTERNAL_LIBRARIES collada14dom xml2)
    ENDIF(COLLADA_USE_STATIC)

ENDIF(WIN32)

SET(TARGET_LIBRARIES_VARS COLLADA_LIBRARY)
SET(TARGET_ADDED_LIBRARIES osgSim ) 

#### end var setup  ###
SETUP_PLUGIN(dae)
