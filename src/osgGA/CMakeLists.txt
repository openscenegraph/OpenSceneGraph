
IF   (DYNAMIC_OPENSCENEGRAPH)
    ADD_DEFINITIONS(-DOSGGA_LIBRARY)
ELSE (DYNAMIC_OPENSCENEGRAPH)
    ADD_DEFINITIONS(-DOSG_LIBRARY_STATIC)
ENDIF(DYNAMIC_OPENSCENEGRAPH)

SET(LIB_NAME osgGA)
SET(HEADER_PATH ${OpenSceneGraph_SOURCE_DIR}/include/${LIB_NAME})
SET(LIB_PUBLIC_HEADERS
    ${HEADER_PATH}/AnimationPathManipulator
    ${HEADER_PATH}/DriveManipulator
    ${HEADER_PATH}/EventQueue
    ${HEADER_PATH}/EventVisitor
    ${HEADER_PATH}/Export
    ${HEADER_PATH}/FlightManipulator
    ${HEADER_PATH}/GUIActionAdapter
    ${HEADER_PATH}/GUIEventAdapter
    ${HEADER_PATH}/GUIEventHandler
    ${HEADER_PATH}/KeySwitchMatrixManipulator
    ${HEADER_PATH}/MatrixManipulator
    ${HEADER_PATH}/NodeTrackerManipulator
    ${HEADER_PATH}/StateSetManipulator
    ${HEADER_PATH}/TerrainManipulator
    ${HEADER_PATH}/TrackballManipulator
    ${HEADER_PATH}/UFOManipulator
    ${HEADER_PATH}/Version
    ${HEADER_PATH}/CameraViewSwitchManipulator
)

# FIXME: For OS X, need flag for Framework or dylib
ADD_LIBRARY(${LIB_NAME}
    ${OPENSCENEGRAPH_USER_DEFINED_DYNAMIC_OR_STATIC}
    ${LIB_PUBLIC_HEADERS}
    AnimationPathManipulator.cpp
    DriveManipulator.cpp
    EventQueue.cpp
    EventVisitor.cpp
    FlightManipulator.cpp
    GUIEventAdapter.cpp
    GUIEventHandler.cpp
    KeySwitchMatrixManipulator.cpp
    MatrixManipulator.cpp
    NodeTrackerManipulator.cpp
    StateSetManipulator.cpp
    TerrainManipulator.cpp
    TrackballManipulator.cpp
    UFOManipulator.cpp
    Version.cpp
    CameraViewSwitchManipulator.cpp
)

LINK_INTERNAL(${LIB_NAME}     
    osgDB
    osgUtil
    osg
    OpenThreads
)

LINK_CORELIB_DEFAULT(${LIB_NAME})

INCLUDE(ModuleInstall OPTIONAL)
