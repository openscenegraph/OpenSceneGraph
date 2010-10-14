
IF   (DYNAMIC_OPENSCENEGRAPH)

    OPTION(OSG_PLUGIN_SEARCH_INSTALL_DIR_FOR_PLUGINS "Set to ON to have OpenSceneGraph search the configured install directory for plugins." ON)

    ADD_DEFINITIONS(-DOSGDB_LIBRARY)

    IF(OSG_PLUGIN_SEARCH_INSTALL_DIR_FOR_PLUGINS)
        # Add a default plugin search path component
        ADD_DEFINITIONS(-DOSG_DEFAULT_LIBRARY_PATH=${CMAKE_INSTALL_PREFIX}/lib${LIB_POSTFIX}/${OSG_PLUGINS})
    ENDIF()

    # Set the library extension according to what configuration is being built.
    # If the string is empty, don't set the define.
    IF(CMAKE_DEBUG_POSTFIX)
        SET(CMAKE_CXX_FLAGS_DEBUG
            "${CMAKE_CXX_FLAGS_DEBUG} -DOSG_LIBRARY_POSTFIX=${CMAKE_DEBUG_POSTFIX}")
    ENDIF()
    IF(CMAKE_RELEASE_POSTFIX)
        SET(CMAKE_CXX_FLAGS_RELEASE
            "${CMAKE_CXX_FLAGS_RELEASE} -DOSG_LIBRARY_POSTFIX=${CMAKE_RELEASE_POSTFIX}")
    ENDIF()
    IF(CMAKE_RELWITHDEBINFO_POSTFIX)
        SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO
            "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -DOSG_LIBRARY_POSTFIX=${CMAKE_RELWITHDEBINFO_POSTFIX}")
    ENDIF()
    IF(CMAKE_MINSIZEREL_POSTFIX)
        SET(CMAKE_CXX_FLAGS_MINSIZEREL
            "${CMAKE_CXX_FLAGS_MINSIZEREL} -DOSG_LIBRARY_POSTFIX=${CMAKE_MINSIZEREL_POSTFIX}")
    ENDIF()
ELSE ()
    ADD_DEFINITIONS(-DOSG_LIBRARY_STATIC)
ENDIF()



SET(LIB_NAME osgDB)
SET(HEADER_PATH ${OpenSceneGraph_SOURCE_DIR}/include/${LIB_NAME})
SET(LIB_PUBLIC_HEADERS
    ${HEADER_PATH}/DataTypes
    ${HEADER_PATH}/StreamOperator
    ${HEADER_PATH}/Serializer
    ${HEADER_PATH}/ObjectWrapper
    ${HEADER_PATH}/InputStream
    ${HEADER_PATH}/OutputStream
    ${HEADER_PATH}/Archive
    ${HEADER_PATH}/AuthenticationMap
    ${HEADER_PATH}/Callbacks
    ${HEADER_PATH}/ConvertUTF
    ${HEADER_PATH}/DatabasePager
    ${HEADER_PATH}/DatabaseRevisions
    ${HEADER_PATH}/DotOsgWrapper
    ${HEADER_PATH}/DynamicLibrary
    ${HEADER_PATH}/Export
    ${HEADER_PATH}/FileCache
    ${HEADER_PATH}/FileNameUtils
    ${HEADER_PATH}/FileUtils
    ${HEADER_PATH}/fstream
    ${HEADER_PATH}/ImageOptions
    ${HEADER_PATH}/ImagePager
    ${HEADER_PATH}/Input
    ${HEADER_PATH}/Output
    ${HEADER_PATH}/Options
    ${HEADER_PATH}/ParameterOutput
    ${HEADER_PATH}/PluginQuery
    ${HEADER_PATH}/ReaderWriter
    ${HEADER_PATH}/ReadFile
    ${HEADER_PATH}/Registry
    ${HEADER_PATH}/SharedStateManager
    ${HEADER_PATH}/Version
    ${HEADER_PATH}/WriteFile
    ${HEADER_PATH}/XmlParser
)

# FIXME: For OS X, need flag for Framework or dylib
ADD_LIBRARY(${LIB_NAME}
    ${OPENSCENEGRAPH_USER_DEFINED_DYNAMIC_OR_STATIC}
    ${LIB_PUBLIC_HEADERS}
    ObjectWrapper.cpp
    InputStream.cpp
    OutputStream.cpp
    Compressors.cpp
    Archive.cpp
    AuthenticationMap.cpp
    Callbacks.cpp
    ConvertUTF.cpp
    DatabasePager.cpp
    DatabaseRevisions.cpp
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
    Options.cpp
    PluginQuery.cpp
    ReaderWriter.cpp
    ReadFile.cpp
    Registry.cpp
    SharedStateManager.cpp
    Version.cpp
    WriteFile.cpp
    XmlParser.cpp
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

IF( ZLIB_FOUND )
    ADD_DEFINITIONS( -DUSE_ZLIB )
    INCLUDE_DIRECTORIES( ${ZLIB_INCLUDE_DIR} )
    SET(COMPRESSION_LIBRARIES ZLIB_LIBRARY)
ENDIF()

ADD_DEFINITIONS(-DOSG_PLUGIN_EXTENSION=${CMAKE_SHARED_MODULE_SUFFIX})

LINK_INTERNAL(${LIB_NAME}     
    osg
    osgUtil
    OpenThreads
)
LINK_EXTERNAL(${LIB_NAME} ${OSGDB_PLATFORM_SPECIFIC_LIBRARIES} ${DL_LIBRARY})
LINK_WITH_VARIABLES(${LIB_NAME} ${COMPRESSION_LIBRARIES})
LINK_CORELIB_DEFAULT(${LIB_NAME})

IF( ZLIB_FOUND )
    SET( TARGET_LIBRARIES_VARS ZLIB_LIBRARY )
ENDIF( ZLIB_FOUND )

INCLUDE(ModuleInstall OPTIONAL)
