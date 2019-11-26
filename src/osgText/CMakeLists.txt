IF(OSG_TEXT_USE_FONTCONFIG)
    find_package(Fontconfig MODULE)
ENDIF()

IF(DYNAMIC_OPENSCENEGRAPH)
    ADD_DEFINITIONS(-DOSGTEXT_LIBRARY)
ELSE()
    ADD_DEFINITIONS(-DOSG_LIBRARY_STATIC)
ENDIF()

SET(LIB_NAME osgText)
SET(HEADER_PATH ${OpenSceneGraph_SOURCE_DIR}/include/${LIB_NAME})
SET(TARGET_H
    ${HEADER_PATH}/Export
    ${HEADER_PATH}/Font
    ${HEADER_PATH}/Font3D
    ${HEADER_PATH}/FadeText
    ${HEADER_PATH}/Glyph
    ${HEADER_PATH}/KerningType
    ${HEADER_PATH}/String
    ${HEADER_PATH}/Style
    ${HEADER_PATH}/TextBase
    ${HEADER_PATH}/Text
    ${HEADER_PATH}/Text3D
    ${HEADER_PATH}/Version
)

# FIXME: For OS X, need flag for Framework or dylib
SET(TARGET_SRC
    DefaultFont.cpp
    DefaultFont.h
    GlyphGeometry.h
    GlyphGeometry.cpp
    Font.cpp
    FadeText.cpp
    Glyph.cpp
    String.cpp
    Style.cpp
    TextBase.cpp
    Text.cpp
    Text3D.cpp
    Version.cpp
    ${OPENSCENEGRAPH_VERSIONINFO_RC}
)

SET(TARGET_LIBRARIES
    osgDB
    osg
    osgUtil
    OpenThreads
)

if(OSG_TEXT_USE_FONTCONFIG AND Fontconfig_FOUND)
    MESSAGE(STATUS "osgText will be linked with FontConfig library")
    LIST(APPEND TARGET_EXTERNAL_LIBRARIES Fontconfig::Fontconfig)
    ADD_DEFINITIONS(-DWITH_FONTCONFIG)
else()
    MESSAGE(STATUS "osgText will not be linked with FontConfig library")
endif()

SETUP_LIBRARY(${LIB_NAME})

