
IF(DYNAMIC_OPENSCENEGRAPH)
    ADD_DEFINITIONS(-DOSGSIM_LIBRARY)
ELSE()
    ADD_DEFINITIONS(-DOSG_LIBRARY_STATIC)
ENDIF()

SET(LIB_NAME osgSim)
SET(HEADER_PATH ${OpenSceneGraph_SOURCE_DIR}/include/${LIB_NAME})
SET(TARGET_H
    ${HEADER_PATH}/BlinkSequence
    ${HEADER_PATH}/ColorRange
    ${HEADER_PATH}/DOFTransform
    ${HEADER_PATH}/ElevationSlice
    ${HEADER_PATH}/HeightAboveTerrain
    ${HEADER_PATH}/Export
    ${HEADER_PATH}/GeographicLocation
    ${HEADER_PATH}/Impostor
    ${HEADER_PATH}/ImpostorSprite
    ${HEADER_PATH}/InsertImpostorsVisitor
    ${HEADER_PATH}/LightPoint
    ${HEADER_PATH}/LightPointNode
    ${HEADER_PATH}/LightPointSystem
    ${HEADER_PATH}/LineOfSight
    ${HEADER_PATH}/MultiSwitch
    ${HEADER_PATH}/OverlayNode
    ${HEADER_PATH}/ObjectRecordData
    ${HEADER_PATH}/ScalarBar
    ${HEADER_PATH}/ScalarsToColors
    ${HEADER_PATH}/Sector
    ${HEADER_PATH}/ShapeAttribute
    ${HEADER_PATH}/SphereSegment
    ${HEADER_PATH}/Version
    ${HEADER_PATH}/VisibilityGroup
)

# FIXME: For OS X, need flag for Framework or dylib
SET(TARGET_SRC
    BlinkSequence.cpp
    ColorRange.cpp
    DOFTransform.cpp
    ElevationSlice.cpp
    HeightAboveTerrain.cpp
    Impostor.cpp
    ImpostorSprite.cpp
    InsertImpostorsVisitor.cpp
    LightPoint.cpp
    LightPointDrawable.cpp
    LightPointDrawable.h
    LightPointNode.cpp
    LightPointSpriteDrawable.cpp
    LightPointSpriteDrawable.h
    LineOfSight.cpp
    MultiSwitch.cpp
    OverlayNode.cpp
    ScalarBar.cpp
    ScalarsToColors.cpp
    Sector.cpp
    ShapeAttribute.cpp
    SphereSegment.cpp
    Version.cpp
    VisibilityGroup.cpp
    ${OPENSCENEGRAPH_VERSIONINFO_RC}
)

SET(TARGET_LIBRARIES
    osgText
    osgUtil
    osgDB
    osg
    OpenThreads
)

SETUP_LIBRARY(${LIB_NAME})


