IF   (DYNAMIC_OPENSCENEGRAPH)
    ADD_DEFINITIONS(-DOSGTERRAIN_LIBRARY)
ELSE (DYNAMIC_OPENSCENEGRAPH)
    ADD_DEFINITIONS(-DOSG_LIBRARY_STATIC)
ENDIF(DYNAMIC_OPENSCENEGRAPH)

SET(LIB_NAME osgTerrain)
SET(HEADER_PATH ${OpenSceneGraph_SOURCE_DIR}/include/${LIB_NAME})
SET(LIB_PUBLIC_HEADERS
    ${HEADER_PATH}/Export
    ${HEADER_PATH}/Locator
    ${HEADER_PATH}/Layer
    ${HEADER_PATH}/TerrainTile
    ${HEADER_PATH}/TerrainTechnique
    ${HEADER_PATH}/Terrain
    ${HEADER_PATH}/GeometryTechnique
    ${HEADER_PATH}/ValidDataOperator
    ${HEADER_PATH}/Version
)

# FIXME: For OS X, need flag for Framework or dylib
ADD_LIBRARY(${LIB_NAME}
    ${OPENSCENEGRAPH_USER_DEFINED_DYNAMIC_OR_STATIC}
    ${LIB_PUBLIC_HEADERS}
    Layer.cpp
    Locator.cpp
    TerrainTile.cpp
    TerrainTechnique.cpp
    Terrain.cpp
    GeometryTechnique.cpp
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


