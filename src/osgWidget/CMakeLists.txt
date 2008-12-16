IF   (DYNAMIC_OPENSCENEGRAPH)
    ADD_DEFINITIONS(-DOSGWIDGET_LIBRARY)
ELSE (DYNAMIC_OPENSCENEGRAPH)
    ADD_DEFINITIONS(-DOSG_LIBRARY_STATIC)
ENDIF(DYNAMIC_OPENSCENEGRAPH)

SET(LIB_NAME osgWidget)
SET(HEADER_PATH ${OpenSceneGraph_SOURCE_DIR}/include/${LIB_NAME})
SET(LIB_PUBLIC_HEADERS
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
ADD_LIBRARY(${LIB_NAME}
    ${OPENSCENEGRAPH_USER_DEFINED_DYNAMIC_OR_STATIC}
    ${LIB_PUBLIC_HEADERS}
    Box.cpp
    Browser.cpp
    PdfReader.cpp
    VncClient.cpp
    Canvas.cpp
    CMakeLists.txt
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
)


SET(TARGET_LIBRARIES_VARS FREETYPE_LIBRARY )
LINK_INTERNAL(${LIB_NAME}     
    osgText
    osgViewer
    osgDB
    osg
    OpenThreads
    osgGA
)
LINK_CORELIB_DEFAULT(${LIB_NAME})

INCLUDE(ModuleInstall OPTIONAL)
