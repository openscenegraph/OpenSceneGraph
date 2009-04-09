
IF   (DYNAMIC_OPENSCENEGRAPH)

    OPTION(OSG_PLUGIN_SEARCH_INSTALL_DIR_FOR_PLUGINS "Set to ON to have OpenSceneGraph search the configured install directory for plugins." ON)

    ADD_DEFINITIONS(-DOSGDB_LIBRARY)

    IF(OSG_PLUGIN_SEARCH_INSTALL_DIR_FOR_PLUGINS)
        # Add a default plugin search path component
        ADD_DEFINITIONS(-DOSG_DEFAULT_LIBRARY_PATH=${CMAKE_INSTALL_PREFIX}/lib${LIB_POSTFIX}/${OSG_PLUGINS})
    ENDIF()

ELSE ()
    ADD_DEFINITIONS(-DOSG_LIBRARY_STATIC)
ENDIF()



SET(LIB_NAME osgDB)
SET(HEADER_PATH ${OpenSceneGraph_SOURCE_DIR}/include/${LIB_NAME})
SET(LIB_PUBLIC_HEADERS
    ${HEADER_PATH}/Archive
    ${HEADER_PATH}/AuthenticationMap
    ${HEADER_PATH}/ConvertUTF
    ${HEADER_PATH}/DatabasePager
    ${HEADER_PATH}/DotOsgWrapper
    ${HEADER_PATH}/DynamicLibrary
    ${HEADER_PATH}/Export
    ${HEADER_PATH}/Field
    ${HEADER_PATH}/FieldReader
    ${HEADER_PATH}/FieldReaderIterator
    ${HEADER_PATH}/FileCache
    ${HEADER_PATH}/FileNameUtils
    ${HEADER_PATH}/FileUtils
    ${HEADER_PATH}/fstream
    ${HEADER_PATH}/ImageOptions
    ${HEADER_PATH}/ImagePager
    ${HEADER_PATH}/Input
    ${HEADER_PATH}/Output
    ${HEADER_PATH}/ParameterOutput
    ${HEADER_PATH}/PluginQuery
    ${HEADER_PATH}/ReaderWriter
    ${HEADER_PATH}/ReadFile
    ${HEADER_PATH}/Registry
    ${HEADER_PATH}/Serializer
    ${HEADER_PATH}/SharedStateManager
    ${HEADER_PATH}/Version
    ${HEADER_PATH}/WriteFile
)

# FIXME: For OS X, need flag for Framework or dylib
ADD_LIBRARY(${LIB_NAME}
    ${OPENSCENEGRAPH_USER_DEFINED_DYNAMIC_OR_STATIC}
    ${LIB_PUBLIC_HEADERS}
    Archive.cpp
    AuthenticationMap.cpp
    ConvertUTF.cpp
    DatabasePager.cpp
    DotOsgWrapper.cpp
    DynamicLibrary.cpp
    Field.cpp
    FieldReader.cpp
    FieldReaderIterator.cpp
    FileCache.cpp
    FileNameUtils.cpp
    FileUtils.cpp
    fstream.cpp
    ImageOptions.cpp
    ImagePager.cpp
    Input.cpp
    MimeTypes.cpp
    Output.cpp
    PluginQuery.cpp
    ReaderWriter.cpp
    ReadFile.cpp
    Registry.cpp
    SharedStateManager.cpp
    Version.cpp
    WriteFile.cpp
    ${OPENSCENEGRAPH_VERSIONINFO_RC}
)

IF(APPLE)
    # Needs CoreFoundation calls and a Carbon function
    SET(OSGDB_PLATFORM_SPECIFIC_LIBRARIES ${CARBON_LIBRARY})
 
    SET(OSG_DEFAULT_IMAGE_PLUGIN_FOR_OSX "quicktime" CACHE STRING "standard image plugin for os x, options are quicktime, imageio")

    IF(OSG_DEFAULT_IMAGE_PLUGIN_FOR_OSX STREQUAL "quicktime")
        ADD_DEFINITIONS(-DDARWIN_QUICKTIME)
    ELSE()
        ADD_DEFINITIONS(-DDARWIN_IMAGEIO)
    ENDIF()
ENDIF()

IF(QUICKTIME_FOUND)
    ADD_DEFINITIONS(-DUSE_QUICKTIME)
ENDIF()

IF(XINE_FOUND)
    ADD_DEFINITIONS(-DUSE_XINE)
ENDIF()

IF(INVENTOR_FOUND)
    ADD_DEFINITIONS(-DUSE_INVENTOR)
ENDIF()

IF(OPENVRML_FOUND)
    ADD_DEFINITIONS(-DUSE_VRML)
ENDIF()


ADD_DEFINITIONS(-DOSG_PLUGIN_EXTENSION=${CMAKE_SHARED_MODULE_SUFFIX})

LINK_INTERNAL(${LIB_NAME}     
    osg
    OpenThreads
)
LINK_EXTERNAL(${LIB_NAME}     ${OSGDB_PLATFORM_SPECIFIC_LIBRARIES} )
LINK_CORELIB_DEFAULT(${LIB_NAME})

INCLUDE(ModuleInstall OPTIONAL)
