
IF   (DYNAMIC_OPENSCENEGRAPH)
	ADD_DEFINITIONS(-DOSGDB_LIBRARY)
ELSE (DYNAMIC_OPENSCENEGRAPH)
	ADD_DEFINITIONS(-DOSG_LIBRARY_STATIC)
ENDIF(DYNAMIC_OPENSCENEGRAPH)

SET(LIB_NAME osgDB)
SET(HEADER_PATH ${OpenSceneGraph_SOURCE_DIR}/include/${LIB_NAME})
SET(LIB_PUBLIC_HEADERS
	${HEADER_PATH}/Archive
	${HEADER_PATH}/DatabasePager
	${HEADER_PATH}/DotOsgWrapper
	${HEADER_PATH}/DynamicLibrary
	${HEADER_PATH}/Export
	${HEADER_PATH}/Field
	${HEADER_PATH}/FieldReader
	${HEADER_PATH}/FieldReaderIterator
	${HEADER_PATH}/FileNameUtils
	${HEADER_PATH}/FileUtils
	${HEADER_PATH}/ImageOptions
	${HEADER_PATH}/Input
	${HEADER_PATH}/Output
	${HEADER_PATH}/ParameterOutput
	${HEADER_PATH}/ReaderWriter
	${HEADER_PATH}/ReadFile
	${HEADER_PATH}/Registry
	${HEADER_PATH}/SharedStateManager
	${HEADER_PATH}/Version
	${HEADER_PATH}/WriteFile
)

# FIXME: For OS X, need flag for Framework or dylib
ADD_LIBRARY(${LIB_NAME}
	${OPENSCENEGRAPH_USER_DEFINED_DYNAMIC_OR_STATIC}
	${LIB_PUBLIC_HEADERS}
	Archive.cpp
	DatabasePager.cpp
	DotOsgWrapper.cpp
	DynamicLibrary.cpp
	Field.cpp
	FieldReader.cpp
	FieldReaderIterator.cpp
	FileNameUtils.cpp
	FileUtils.cpp
	ImageOptions.cpp
	Input.cpp
	Output.cpp
	ReadFile.cpp
	ReaderWriter.cpp
	Registry.cpp
	SharedStateManager.cpp
	Version.cpp
	WriteFile.cpp
)

IF(APPLE)
	# Needs CoreFoundation calls and a Carbon function
	SET(OSGDB_PLATFORM_SPECIFIC_LIBRARIES ${CARBON_LIBRARY})
ENDIF(APPLE)

TARGET_LINK_LIBRARIES(${LIB_NAME}
	osg
	${OPENTHREADS_LIBRARY}
	${OPENGL_LIBRARIES}
	${OSGDB_PLATFORM_SPECIFIC_LIBRARIES}
)

INCLUDE(ModuleInstall OPTIONAL)
