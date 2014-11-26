IF(DYNAMIC_OPENSCENEGRAPH)
    ADD_DEFINITIONS(-DOSGTERRAIN_LIBRARY)
ELSE()
    ADD_DEFINITIONS(-DOSG_LIBRARY_STATIC)
ENDIF()

SET(LIB_NAME osgTerrain)
SET(HEADER_PATH ${OpenSceneGraph_SOURCE_DIR}/include/${LIB_NAME})
SET(TARGET_H
    ${HEADER_PATH}/DisplacementMappingTechnique
    ${HEADER_PATH}/Export
    ${HEADER_PATH}/Locator
    ${HEADER_PATH}/Layer
    ${HEADER_PATH}/TerrainTile
    ${HEADER_PATH}/TerrainTechnique
    ${HEADER_PATH}/Terrain
    ${HEADER_PATH}/GeometryTechnique
    ${HEADER_PATH}/GeometryPool
    ${HEADER_PATH}/ValidDataOperator
    ${HEADER_PATH}/Version
)

# FIXME: For OS X, need flag for Framework or dylib
SET(TARGET_SRC
    DisplacementMappingTechnique.cpp
    Layer.cpp
    Locator.cpp
    TerrainTile.cpp
    TerrainTechnique.cpp
    Terrain.cpp
    GeometryTechnique.cpp
    GeometryPool.cpp
    Version.cpp
    ${OPENSCENEGRAPH_VERSIONINFO_RC}
)

SET(TARGET_LIBRARIES
    osgUtil
    osgDB
    osg
    OpenThreads
)

SETUP_LIBRARY(${LIB_NAME})

