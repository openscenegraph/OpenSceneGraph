
IF   (DYNAMIC_OPENSCENEGRAPH)
    ADD_DEFINITIONS(-DOSGUI_LIBRARY)
ELSE (DYNAMIC_OPENSCENEGRAPH)
    ADD_DEFINITIONS(-DOSG_LIBRARY_STATIC)
ENDIF(DYNAMIC_OPENSCENEGRAPH)

SET(LIB_NAME osgUI)
SET(HEADER_PATH ${OpenSceneGraph_SOURCE_DIR}/include/${LIB_NAME})
SET(TARGET_H
    ${HEADER_PATH}/Export
    ${HEADER_PATH}/ColorPalette
    ${HEADER_PATH}/Widget
    ${HEADER_PATH}/Label
    ${HEADER_PATH}/LineEdit
    ${HEADER_PATH}/Dialog
    ${HEADER_PATH}/Popup
    ${HEADER_PATH}/PushButton
    ${HEADER_PATH}/ComboBox
    ${HEADER_PATH}/Callbacks
    ${HEADER_PATH}/Style
    ${HEADER_PATH}/AlignmentSettings
    ${HEADER_PATH}/FrameSettings
    ${HEADER_PATH}/TabWidget
    ${HEADER_PATH}/TextSettings
    ${HEADER_PATH}/Validator
)

SET(TARGET_SRC
    Widget.cpp
    ColorPalette.cpp
    Label.cpp
    LineEdit.cpp
    Dialog.cpp
    Popup.cpp
    PushButton.cpp
    ComboBox.cpp
    Callbacks.cpp
    Style.cpp
    AlignmentSettings.cpp
    FrameSettings.cpp
    TabWidget.cpp
    TextSettings.cpp
    Validator.cpp
    ${OPENSCENEGRAPH_VERSIONINFO_RC}
)

SET(TARGET_LIBRARIES
    osgDB
    osgGA
    osgUtil
    osgText
    osgViewer
    osg
    OpenThreads
)

SETUP_LIBRARY(${LIB_NAME})

