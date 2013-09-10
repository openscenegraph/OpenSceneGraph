
IF(DYNAMIC_OPENSCENEGRAPH)
    ADD_DEFINITIONS(-DOSGPRESENTATION_LIBRARY)
ELSE()
    ADD_DEFINITIONS(-DOSG_LIBRARY_STATIC)
ENDIF()

SET(LIB_NAME osgPresentation)
SET(HEADER_PATH ${OpenSceneGraph_SOURCE_DIR}/include/${LIB_NAME})
SET(TARGET_H
    ${HEADER_PATH}/Export

    ${HEADER_PATH}/Action
    ${HEADER_PATH}/Cursor
    ${HEADER_PATH}/Group

    ${HEADER_PATH}/Presentation
    ${HEADER_PATH}/Section
    ${HEADER_PATH}/Slide
    ${HEADER_PATH}/Layer

    ${HEADER_PATH}/Element
    ${HEADER_PATH}/Audio
    ${HEADER_PATH}/Image
    ${HEADER_PATH}/Model
    ${HEADER_PATH}/Movie
    ${HEADER_PATH}/Text
    ${HEADER_PATH}/Volume

    ${HEADER_PATH}/deprecated/AnimationMaterial
    ${HEADER_PATH}/deprecated/CompileSlideCallback
    ${HEADER_PATH}/deprecated/PickEventHandler
    ${HEADER_PATH}/deprecated/PropertyManager
    ${HEADER_PATH}/deprecated/KeyEventHandler
    ${HEADER_PATH}/deprecated/SlideEventHandler
    ${HEADER_PATH}/deprecated/SlideShowConstructor
    ${HEADER_PATH}/deprecated/Timeout
)

# FIXME: For OS X, need flag for Framework or dylib
SET(TARGET_SRC
    Action.cpp
    Cursor.cpp
    Group.cpp

    Show.cpp
    Presentation.cpp
    Section.cpp
    Slide.cpp
    Layer.cpp

    Element.cpp
    Audio.cpp
    Image.cpp
    Model.cpp
    Movie.cpp
    Text.cpp
    Volume.cpp

    deprecated/AnimationMaterial.cpp
    deprecated/CompileSlideCallback.cpp
    deprecated/PickEventHandler.cpp
    deprecated/PropertyManager.cpp
    deprecated/KeyEventHandler.cpp
    deprecated/SlideEventHandler.cpp
    deprecated/SlideShowConstructor.cpp
    deprecated/Timeout.cpp
    ${OPENSCENEGRAPH_VERSIONINFO_RC}
)

SET(TARGET_LIBRARIES
    osgViewer
    osgManipulator
    osgVolume
    osgFX
    osgText
    osgGA
    osgUtil
    osgDB
    osg
    OpenThreads
)

SETUP_LIBRARY(${LIB_NAME})

