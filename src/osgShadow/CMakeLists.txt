
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

    ${HEADER_PATH}/ConvexPolyhedron
    ${HEADER_PATH}/DebugShadowMap
    ${HEADER_PATH}/LightSpacePerspectiveShadowMap
    ${HEADER_PATH}/MinimalCullBoundsShadowMap
    ${HEADER_PATH}/MinimalDrawBoundsShadowMap
    ${HEADER_PATH}/MinimalShadowMap
    ${HEADER_PATH}/ProjectionShadowMap
    ${HEADER_PATH}/StandardShadowMap
    ${HEADER_PATH}/ViewDependentShadowTechnique
)

# FIXME: For OS X, need flag for Framework or dylib
ADD_LIBRARY(${LIB_NAME}
    ${OPENSCENEGRAPH_USER_DEFINED_DYNAMIC_OR_STATIC}
    ${LIB_PUBLIC_HEADERS}
    OccluderGeometry.cpp
    ShadowMap.cpp
    ShadowTechnique.cpp
    ShadowTexture.cpp
    ShadowVolume.cpp
    ShadowedScene.cpp
    SoftShadowMap.cpp
    ParallelSplitShadowMap.cpp
    Version.cpp
    ConvexPolyhedron.cpp
    DebugShadowMap.cpp
    LightSpacePerspectiveShadowMap.cpp
    MinimalCullBoundsShadowMap.cpp
    MinimalDrawBoundsShadowMap.cpp
    MinimalShadowMap.cpp
    StandardShadowMap.cpp
    ViewDependentShadowTechnique.cpp
)

LINK_INTERNAL(${LIB_NAME}     
    osgUtil
    osgDB
    osg
    OpenThreads
)
LINK_CORELIB_DEFAULT(${LIB_NAME})

INCLUDE(ModuleInstall OPTIONAL)
