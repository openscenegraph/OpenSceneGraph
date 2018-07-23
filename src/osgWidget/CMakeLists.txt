IF(DYNAMIC_OPENSCENEGRAPH)
    ADD_DEFINITIONS(-DOSGWIDGET_LIBRARY)
ELSE()
    ADD_DEFINITIONS(-DOSG_LIBRARY_STATIC)
ENDIF()

################################################################################
## Quieten warnings that a due to optional code paths

IF(CMAKE_COMPILER_IS_GNUCXX)
    # Remove -Wextra flag as it barfs on ffmoeg headers
   STRING(REGEX REPLACE "-Wextra" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
ENDIF()

SET(LIB_NAME osgWidget)
SET(HEADER_PATH ${OpenSceneGraph_SOURCE_DIR}/include/${LIB_NAME})
SET(TARGET_H
    ${HEADER_PATH}/Export
    ${HEADER_PATH}/Box
    ${HEADER_PATH}/Browser
    ${HEADER_PATH}/PdfReader
    ${HEADER_PATH}/VncClient
    ${HEADER_PATH}/Canvas
    ${HEADER_PATH}/EventInterface
    ${HEADER_PATH}/Frame
    ${HEADER_PATH}/Input
    ${HEADER_PATH}/Label
    ${HEADER_PATH}/Lua
    ${HEADER_PATH}/Python
    ${HEADER_PATH}/ScriptEngine
    ${HEADER_PATH}/StyleInterface
    ${HEADER_PATH}/StyleManager
    ${HEADER_PATH}/Table
    ${HEADER_PATH}/Types
    ${HEADER_PATH}/UIObjectParent
    ${HEADER_PATH}/Util
    ${HEADER_PATH}/Version
    ${HEADER_PATH}/ViewerEventHandlers
    ${HEADER_PATH}/Widget
    ${HEADER_PATH}/Window
    ${HEADER_PATH}/WindowManager
)

# FIXME: For OS X, need flag for Framework or dylib
SET(TARGET_SRC
    Box.cpp
    Browser.cpp
    PdfReader.cpp
    VncClient.cpp
    Canvas.cpp
    Frame.cpp
    Input.cpp
    Label.cpp
    Lua.cpp
    Python.cpp
    StyleManager.cpp
    Table.cpp
    Util.cpp
    Version.cpp
    ViewerEventHandlers.cpp
    Widget.cpp
    Window.cpp
    WindowManager.cpp
    ${OPENSCENEGRAPH_VERSIONINFO_RC}
)


SET(TARGET_LIBRARIES
    osgText
    osgViewer
    osgDB
    osg
    OpenThreads
    osgGA
)

SETUP_LIBRARY(${LIB_NAME})

