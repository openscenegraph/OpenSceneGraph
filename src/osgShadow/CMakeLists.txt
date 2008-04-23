
IF   (DYNAMIC_OPENSCENEGRAPH)
    ADD_DEFINITIONS(-DOSGSHADOW_LIBRARY)
ELSE (DYNAMIC_OPENSCENEGRAPH)
    ADD_DEFINITIONS(-DOSG_LIBRARY_STATIC)
ENDIF(DYNAMIC_OPENSCENEGRAPH)

SET(LIB_NAME osgShadow)
SET(HEADER_PATH ${OpenSceneGraph_SOURCE_DIR}/include/${LIB_NAME})
SET(LIB_PUBLIC_HEADERS
    ${HEADER_PATH}/Export
    ${HEADER_PATH}/OccluderGeometry
    ${HEADER_PATH}/ShadowMap
    ${HEADER_PATH}/ShadowTechnique
    ${HEADER_PATH}/ShadowTexture
    ${HEADER_PATH}/ShadowVolume
    ${HEADER_PATH}/ShadowedScene
    ${HEADER_PATH}/SoftShadowMap
    ${HEADER_PATH}/ParallelSplitShadowMap
    ${HEADER_PATH}/Version
)

IF(APPLE)
    SET(ADD_LIBRARY_HEADERS "")
ELSEIF(APPLE)
    SET(ADD_LIBRARY_HEADERS ${LIB_PUBLIC_HEADERS} )
ENDIF(APPLE)

# FIXME: For OS X, need flag for Framework or dylib
ADD_LIBRARY(${LIB_NAME}
    ${OPENSCENEGRAPH_USER_DEFINED_DYNAMIC_OR_STATIC}
    ${ADD_LIBRARY_HEADERS}
    OccluderGeometry.cpp
    ShadowMap.cpp
    ShadowTechnique.cpp
    ShadowTexture.cpp
    ShadowVolume.cpp
    ShadowedScene.cpp
    SoftShadowMap.cpp
    ParallelSplitShadowMap.cpp
    Version.cpp
)

LINK_INTERNAL(${LIB_NAME}     
    osgUtil
    osgDB
    osg
    OpenThreads
)
LINK_CORELIB_DEFAULT(${LIB_NAME})

INCLUDE(ModuleInstall OPTIONAL)
