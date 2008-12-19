
IF   (DYNAMIC_OPENSCENEGRAPH)
    ADD_DEFINITIONS(-DOSGTEXT_LIBRARY)
ELSE (DYNAMIC_OPENSCENEGRAPH)
    ADD_DEFINITIONS(-DOSG_LIBRARY_STATIC)
ENDIF(DYNAMIC_OPENSCENEGRAPH)

SET(LIB_NAME osgText)
SET(HEADER_PATH ${OpenSceneGraph_SOURCE_DIR}/include/${LIB_NAME})
SET(LIB_PUBLIC_HEADERS
    ${HEADER_PATH}/Export
    ${HEADER_PATH}/Font
    ${HEADER_PATH}/Font3D
    ${HEADER_PATH}/KerningType
    ${HEADER_PATH}/String
    ${HEADER_PATH}/TextBase
    ${HEADER_PATH}/Text
    ${HEADER_PATH}/Text3D
    ${HEADER_PATH}/FadeText
    ${HEADER_PATH}/Version
)

# FIXME: For OS X, need flag for Framework or dylib
ADD_LIBRARY(${LIB_NAME}
    ${OPENSCENEGRAPH_USER_DEFINED_DYNAMIC_OR_STATIC}
    ${LIB_PUBLIC_HEADERS}
    DefaultFont.cpp
    DefaultFont.h
    Font.cpp
    Font3D.cpp
    String.cpp
    FadeText.cpp
    TextBase.cpp
    Text.cpp
    Text3D.cpp
    Version.cpp
)

LINK_INTERNAL(${LIB_NAME}     
    osgDB
    osg
    OpenThreads
)
LINK_CORELIB_DEFAULT(${LIB_NAME})

INCLUDE(ModuleInstall OPTIONAL)
