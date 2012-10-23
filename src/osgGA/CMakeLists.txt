
IF   (DYNAMIC_OPENSCENEGRAPH)
    ADD_DEFINITIONS(-DOSGGA_LIBRARY)
ELSE (DYNAMIC_OPENSCENEGRAPH)
    ADD_DEFINITIONS(-DOSG_LIBRARY_STATIC)
ENDIF(DYNAMIC_OPENSCENEGRAPH)

SET(LIB_NAME osgGA)
SET(HEADER_PATH ${OpenSceneGraph_SOURCE_DIR}/include/${LIB_NAME})
SET(TARGET_H
    ${HEADER_PATH}/AnimationPathManipulator
    ${HEADER_PATH}/DriveManipulator
    ${HEADER_PATH}/Device
    ${HEADER_PATH}/EventQueue
    ${HEADER_PATH}/EventVisitor
    ${HEADER_PATH}/Export
    ${HEADER_PATH}/FirstPersonManipulator
    ${HEADER_PATH}/FlightManipulator
    ${HEADER_PATH}/GUIActionAdapter
    ${HEADER_PATH}/GUIEventAdapter
    ${HEADER_PATH}/GUIEventHandler
    ${HEADER_PATH}/KeySwitchMatrixManipulator
    ${HEADER_PATH}/CameraManipulator
    ${HEADER_PATH}/MultiTouchTrackballManipulator
    ${HEADER_PATH}/NodeTrackerManipulator
    ${HEADER_PATH}/OrbitManipulator
    ${HEADER_PATH}/StandardManipulator
    ${HEADER_PATH}/SphericalManipulator
    ${HEADER_PATH}/StateSetManipulator
    ${HEADER_PATH}/TerrainManipulator
    ${HEADER_PATH}/TrackballManipulator
    ${HEADER_PATH}/UFOManipulator
    ${HEADER_PATH}/Version
    ${HEADER_PATH}/CameraViewSwitchManipulator
)

SET(TARGET_SRC
    AnimationPathManipulator.cpp
    DriveManipulator.cpp
    Device.cpp
    EventQueue.cpp
    EventVisitor.cpp
    FirstPersonManipulator.cpp
    FlightManipulator.cpp
    GUIEventAdapter.cpp
    GUIEventHandler.cpp
    KeySwitchMatrixManipulator.cpp
    CameraManipulator.cpp
    MultiTouchTrackballManipulator.cpp
    NodeTrackerManipulator.cpp
    OrbitManipulator.cpp
    StandardManipulator.cpp
    SphericalManipulator.cpp
    StateSetManipulator.cpp
    TerrainManipulator.cpp
    TrackballManipulator.cpp
    UFOManipulator.cpp
    Version.cpp
    CameraViewSwitchManipulator.cpp
    ${OPENSCENEGRAPH_VERSIONINFO_RC}
)

SET(TARGET_LIBRARIES
    osgDB
    osgUtil
    osg
    OpenThreads
)

SETUP_LIBRARY(${LIB_NAME})

