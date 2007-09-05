
IF   (DYNAMIC_OPENSCENEGRAPH)
    ADD_DEFINITIONS(-DOSGFX_LIBRARY)
ELSE (DYNAMIC_OPENSCENEGRAPH)
    ADD_DEFINITIONS(-DOSG_LIBRARY_STATIC)
ENDIF(DYNAMIC_OPENSCENEGRAPH)

SET(LIB_NAME osgFX)
SET(HEADER_PATH ${OpenSceneGraph_SOURCE_DIR}/include/${LIB_NAME})
SET(LIB_PUBLIC_HEADERS
    ${HEADER_PATH}/AnisotropicLighting
    ${HEADER_PATH}/BumpMapping
    ${HEADER_PATH}/Cartoon
    ${HEADER_PATH}/Effect
    ${HEADER_PATH}/Export
    ${HEADER_PATH}/MultiTextureControl
    ${HEADER_PATH}/Registry
    ${HEADER_PATH}/Scribe
    ${HEADER_PATH}/SpecularHighlights
    ${HEADER_PATH}/Technique
    ${HEADER_PATH}/Validator
    ${HEADER_PATH}/Version
)

# FIXME: For OS X, need flag for Framework or dylib
ADD_LIBRARY(${LIB_NAME}
    ${OPENSCENEGRAPH_USER_DEFINED_DYNAMIC_OR_STATIC}
    ${LIB_PUBLIC_HEADERS}
    AnisotropicLighting.cpp
    BumpMapping.cpp
    Cartoon.cpp
    Effect.cpp
    MultiTextureControl.cpp
    Registry.cpp
    Scribe.cpp
    SpecularHighlights.cpp
    Technique.cpp
    Validator.cpp
    Version.cpp
)

LINK_INTERNAL(${LIB_NAME}     
    osgUtil
    osgDB
    osg
    OpenThreads
)
LINK_CORELIB_DEFAULT(${LIB_NAME})

INCLUDE(ModuleInstall OPTIONAL)
