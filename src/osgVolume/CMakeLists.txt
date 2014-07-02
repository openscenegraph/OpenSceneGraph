
IF(DYNAMIC_OPENSCENEGRAPH)
    ADD_DEFINITIONS(-DOSGVOLUME_LIBRARY)
ELSE()
    ADD_DEFINITIONS(-DOSG_LIBRARY_STATIC)
ENDIF()

SET(LIB_NAME osgVolume)
SET(HEADER_PATH ${OpenSceneGraph_SOURCE_DIR}/include/${LIB_NAME})
SET(TARGET_H
    ${HEADER_PATH}/Export
    ${HEADER_PATH}/FixedFunctionTechnique
    ${HEADER_PATH}/Layer
    ${HEADER_PATH}/Locator
    ${HEADER_PATH}/MultipassTechnique
    ${HEADER_PATH}/Property
    ${HEADER_PATH}/RayTracedTechnique
    ${HEADER_PATH}/Version
    ${HEADER_PATH}/Volume
    ${HEADER_PATH}/VolumeScene
    ${HEADER_PATH}/VolumeSettings
    ${HEADER_PATH}/VolumeTechnique
    ${HEADER_PATH}/VolumeTile
)

# FIXME: For OS X, need flag for Framework or dylib
SET(TARGET_SRC
    FixedFunctionTechnique.cpp
    Layer.cpp
    Locator.cpp
    MultipassTechnique.cpp
    Property.cpp
    RayTracedTechnique.cpp
    Version.cpp
    Volume.cpp
    VolumeScene.cpp
    VolumeSettings.cpp
    VolumeTechnique.cpp
    VolumeTile.cpp
    ${OPENSCENEGRAPH_VERSIONINFO_RC}
)


SET(TARGET_LIBRARIES
    osgUtil
    osgDB
    osgGA
    osg
    OpenThreads
)

SETUP_LIBRARY(${LIB_NAME})


