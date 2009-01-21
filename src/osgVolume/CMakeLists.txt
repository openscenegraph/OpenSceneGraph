
IF   (DYNAMIC_OPENSCENEGRAPH)
    ADD_DEFINITIONS(-DOSGVOLUME_LIBRARY)
ELSE (DYNAMIC_OPENSCENEGRAPH)
    ADD_DEFINITIONS(-DOSG_LIBRARY_STATIC)
ENDIF(DYNAMIC_OPENSCENEGRAPH)

SET(LIB_NAME osgVolume)
SET(HEADER_PATH ${OpenSceneGraph_SOURCE_DIR}/include/${LIB_NAME})
SET(LIB_PUBLIC_HEADERS
    ${HEADER_PATH}/Export
    ${HEADER_PATH}/FixedFunctionTechnique
    ${HEADER_PATH}/Layer
    ${HEADER_PATH}/Locator
    ${HEADER_PATH}/Property
    ${HEADER_PATH}/RayTracedTechnique
    ${HEADER_PATH}/Version
    ${HEADER_PATH}/Volume
    ${HEADER_PATH}/VolumeTechnique
    ${HEADER_PATH}/VolumeTile
)

# FIXME: For OS X, need flag for Framework or dylib
ADD_LIBRARY(${LIB_NAME}
    ${OPENSCENEGRAPH_USER_DEFINED_DYNAMIC_OR_STATIC}
    ${LIB_PUBLIC_HEADERS}
    FixedFunctionTechnique.cpp
    Layer.cpp
    Locator.cpp
    Property.cpp
    RayTracedTechnique.cpp
    Version.cpp
    Volume.cpp
    VolumeTechnique.cpp
    VolumeTile.cpp
)


SET(TARGET_LIBRARIES_VARS FREETYPE_LIBRARY )
LINK_INTERNAL(${LIB_NAME}     
    osgUtil
    osgDB
    osgGA
    osg
    OpenThreads
)
LINK_CORELIB_DEFAULT(${LIB_NAME})

INCLUDE(ModuleInstall OPTIONAL)
