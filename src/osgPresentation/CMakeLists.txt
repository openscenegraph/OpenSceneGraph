
IF(DYNAMIC_OPENSCENEGRAPH)
    ADD_DEFINITIONS(-DOSGPRESENTATION_LIBRARY)
ELSE()
    ADD_DEFINITIONS(-DOSG_LIBRARY_STATIC)
ENDIF()

SET(LIB_NAME osgPresentation)
SET(HEADER_PATH ${OpenSceneGraph_SOURCE_DIR}/include/${LIB_NAME})
SET(LIB_PUBLIC_HEADERS
    ${HEADER_PATH}/AnimationMaterial
    ${HEADER_PATH}/CompileSlideCallback
    ${HEADER_PATH}/PickEventHandler
    ${HEADER_PATH}/SlideEventHandler
    ${HEADER_PATH}/SlideShowConstructor
)

# FIXME: For OS X, need flag for Framework or dylib
ADD_LIBRARY(${LIB_NAME}
    ${OPENSCENEGRAPH_USER_DEFINED_DYNAMIC_OR_STATIC}
    ${LIB_PUBLIC_HEADERS}
    AnimationMaterial.cpp
    CompileSlideCallback.cpp
    PickEventHandler.cpp
    SlideEventHandler.cpp
    SlideShowConstructor.cpp
    ${OPENSCENEGRAPH_VERSIONINFO_RC}
)

LINK_INTERNAL(${LIB_NAME}
    osgViewer
    osgVolume
    osgFX
    osgText
    osgGA
    osgUtil
    osgDB
    osg
    OpenThreads
)
LINK_CORELIB_DEFAULT(${LIB_NAME})

INCLUDE(ModuleInstall OPTIONAL)
