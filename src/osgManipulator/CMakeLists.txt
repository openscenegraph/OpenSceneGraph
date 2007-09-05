
IF   (DYNAMIC_OPENSCENEGRAPH)
    ADD_DEFINITIONS(-DOSGMANIPULATOR_LIBRARY)
ELSE (DYNAMIC_OPENSCENEGRAPH)
    ADD_DEFINITIONS(-DOSG_LIBRARY_STATIC)
ENDIF(DYNAMIC_OPENSCENEGRAPH)

SET(LIB_NAME osgManipulator)
SET(HEADER_PATH ${OpenSceneGraph_SOURCE_DIR}/include/${LIB_NAME})
SET(LIB_PUBLIC_HEADERS
    ${HEADER_PATH}/AntiSquish
    ${HEADER_PATH}/Command
    ${HEADER_PATH}/CommandManager
    ${HEADER_PATH}/Constraint
    ${HEADER_PATH}/Dragger
    ${HEADER_PATH}/Export
    ${HEADER_PATH}/Projector
    ${HEADER_PATH}/RotateCylinderDragger
    ${HEADER_PATH}/RotateSphereDragger
    ${HEADER_PATH}/Scale1DDragger
    ${HEADER_PATH}/Scale2DDragger
    ${HEADER_PATH}/ScaleAxisDragger
    ${HEADER_PATH}/Selection
    ${HEADER_PATH}/TabBoxDragger
    ${HEADER_PATH}/TabPlaneDragger
    ${HEADER_PATH}/TabPlaneTrackballDragger
    ${HEADER_PATH}/TrackballDragger
    ${HEADER_PATH}/Translate1DDragger
    ${HEADER_PATH}/Translate2DDragger
    ${HEADER_PATH}/TranslateAxisDragger
    ${HEADER_PATH}/TranslatePlaneDragger
    ${HEADER_PATH}/Version
)

# FIXME: For OS X, need flag for Framework or dylib
ADD_LIBRARY(${LIB_NAME}
    ${OPENSCENEGRAPH_USER_DEFINED_DYNAMIC_OR_STATIC}
    ${LIB_PUBLIC_HEADERS}
    AntiSquish.cpp
    Command.cpp
    CommandManager.cpp
    Constraint.cpp
    Dragger.cpp
    Projector.cpp
    RotateCylinderDragger.cpp
    RotateSphereDragger.cpp
    Scale1DDragger.cpp
    Scale2DDragger.cpp
    ScaleAxisDragger.cpp
    Selection.cpp
    TabBoxDragger.cpp
    TabPlaneDragger.cpp
    TabPlaneTrackballDragger.cpp
    TrackballDragger.cpp
    Translate1DDragger.cpp
    Translate2DDragger.cpp
    TranslateAxisDragger.cpp
    TranslatePlaneDragger.cpp
    Version.cpp
)

LINK_INTERNAL(${LIB_NAME}     
    osgGA
    osgUtil
    osg
    OpenThreads
)
LINK_CORELIB_DEFAULT(${LIB_NAME})

INCLUDE(ModuleInstall OPTIONAL)
