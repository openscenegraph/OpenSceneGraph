
IF   (DYNAMIC_OPENSCENEGRAPH)

    OPTION(OSG_PLUGIN_SEARCH_INSTALL_DIR_FOR_PLUGINS "Set to ON to have OpenSceneGraph search the configured install directory for plugins." ON)

    ADD_DEFINITIONS(-DOSGDB_LIBRARY)

    IF(OSG_PLUGIN_SEARCH_INSTALL_DIR_FOR_PLUGINS)
        # Add a default plugin search path component
        ADD_DEFINITIONS(-DOSG_DEFAULT_LIBRARY_PATH=\"${CMAKE_INSTALL_PREFIX}/lib${LIB_POSTFIX}/${OSG_PLUGINS}\")
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

IF(APPLE AND NOT ANDROID)
     # compile FileUtils.cpp as objective-c++
     SET_SOURCE_FILES_PROPERTIES(FileUtils.cpp
       PROPERTIES COMPILE_FLAGS "-x objective-c++"
     )
ENDIF()


SET(LIB_NAME osgDB)
SET(HEADER_PATH ${OpenSceneGraph_SOURCE_DIR}/include/${LIB_NAME})
SET(TARGET_H
    ${HEADER_PATH}/DataTypes
    ${HEADER_PATH}/StreamOperator
    ${HEADER_PATH}/Serializer
    ${HEADER_PATH}/ObjectWrapper
    ${HEADER_PATH}/InputStream
    ${HEADER_PATH}/OutputStream
    ${HEADER_PATH}/Archive
    ${HEADER_PATH}/AuthenticationMap
    ${HEADER_PATH}/Callbacks
    ${HEADER_PATH}/ClassInterface
    ${HEADER_PATH}/ConvertBase64
    ${HEADER_PATH}/ConvertUTF
    ${HEADER_PATH}/DatabasePager
    ${HEADER_PATH}/DatabaseRevisions
    ${HEADER_PATH}/DotOsgWrapper
    ${HEADER_PATH}/DynamicLibrary
    ${HEADER_PATH}/Export
    ${HEADER_PATH}/ExternalFileWriter
    ${HEADER_PATH}/FileCache
    ${HEADER_PATH}/FileNameUtils
    ${HEADER_PATH}/FileUtils
    ${HEADER_PATH}/fstream
    ${HEADER_PATH}/ImageOptions
    ${HEADER_PATH}/ImagePager
    ${HEADER_PATH}/ImageProcessor
    ${HEADER_PATH}/Input
    ${HEADER_PATH}/ObjectCache
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

SET(TARGET_SRC
    ObjectWrapper.cpp
    InputStream.cpp
    OutputStream.cpp
    Compressors.cpp
    Archive.cpp
    AuthenticationMap.cpp
    Callbacks.cpp
    ClassInterface.cpp
    ConvertBase64.cpp
    ConvertUTF.cpp
    DatabasePager.cpp
    DatabaseRevisions.cpp
    DotOsgWrapper.cpp
    DynamicLibrary.cpp
    ExternalFileWriter.cpp
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
    ObjectCache.cpp
    Output.cpp
    Options.cpp
    PluginQuery.cpp
    ReaderWriter.cpp
    ReadFile.cpp
    Registry.cpp
    SharedStateManager.cpp
    StreamOperator.cpp
    Version.cpp
    WriteFile.cpp
    XmlParser.cpp
    ${OPENSCENEGRAPH_VERSIONINFO_RC}
)

IF(APPLE AND NOT ANDROID)
    IF(NOT OSG_BUILD_PLATFORM_IPHONE AND NOT OSG_BUILD_PLATFORM_IPHONE_SIMULATOR)
        # Needs CoreFoundation calls and a Carbon function
        SET(OSGDB_PLATFORM_SPECIFIC_LIBRARIES ${CARBON_LIBRARY} ${COCOA_LIBRARY})
    ENDIF()

    IF(OSG_DEFAULT_IMAGE_PLUGIN_FOR_OSX STREQUAL "quicktime")
        ADD_DEFINITIONS(-DDARWIN_QUICKTIME)
    ELSE()
        ADD_DEFINITIONS(-DDARWIN_IMAGEIO)
    ENDIF()
ENDIF()
IF(QTKIT_FOUND)
    ADD_DEFINITIONS(-DUSE_QTKIT)
ENDIF()
IF(QUICKTIME_FOUND)
    ADD_DEFINITIONS(-DUSE_QUICKTIME)
ENDIF()
IF(AV_FOUNDATION_FOUND)
    ADD_DEFINITIONS(-DUSE_AV_FOUNDATION)
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

SET(TARGET_LIBRARIES
    osg
    osgUtil
    OpenThreads
)
SET(TARGET_EXTERNAL_LIBRARIES ${OSGDB_PLATFORM_SPECIFIC_LIBRARIES} ${DL_LIBRARY})
SET(TARGET_LIBRARIES_VARS  ${COMPRESSION_LIBRARIES})

SETUP_LIBRARY(${LIB_NAME})

