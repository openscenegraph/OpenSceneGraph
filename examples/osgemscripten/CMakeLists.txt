CMAKE_MINIMUM_REQUIRED(VERSION 2.8.0 FATAL_ERROR)

# Path to OpenSceneGraph
SET(OSG_DIR "${CMAKE_SOURCE_DIR}/../..")

# Specify critical OpenSceneGraph build variables.
SET(BUILD_OSG_APPLICATIONS NO CACHE BOOL "Do not build applications")
SET(EGL_LIBRARY "GL" CACHE STRING "Suppress linkage error")
SET(OSG_GL1_AVAILABLE OFF CACHE BOOL "Unavailable under Emscripten")
SET(OSG_GL2_AVAILABLE OFF CACHE BOOL "Unavailable under Emscripten")
SET(OSG_GLES2_AVAILABLE ON CACHE BOOL "GLES2 is what Emscripten uses")
SET(OSG_WINDOWING_SYSTEM "None" CACHE STRING "Unavailable under Emscripten")
SET(DYNAMIC_OPENTHREADS OFF CACHE BOOL "Link OpenThreads statically")
SET(DYNAMIC_OPENSCENEGRAPH OFF CACHE BOOL "Link OpenSceneGraph statically")
# Prevent CMake configuration error.
SET(_OPENTHREADS_ATOMIC_USE_GCC_BUILTINS_EXITCODE "0" CACHE STRING "Prevent cfg error")
# Reference SDL2 during build process.
# We use SDL2 to do the following:
# * OpenGL functions' address retrieval
# * OpenGL graphics context creation
SET(USE_SDL2 "-s USE_SDL=2")
SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${USE_SDL2}" CACHE STRING "Use SDL2" FORCE)

# Reference OpenSceneGraph includes and libraries.
SET(OSG_SOURCE_DIR "${OSG_DIR}")
SET(OSG_BUILD_DIR "${OSG_DIR}/build/Emscripten")
INCLUDE_DIRECTORIES(${OSG_SOURCE_DIR}/include)
INCLUDE_DIRECTORIES(${OSG_BUILD_DIR}/include)
LINK_DIRECTORIES(${OSG_BUILD_DIR}/lib)

# Build OpenSceneGraph if osgViewer library is missing.
IF(EXISTS "${OSG_BUILD_DIR}/lib/libosgViewer.a")
    MESSAGE("Skip building OpenSceneGraph")
ELSE()
    MESSAGE("Build OpenSceneGraph")
    FILE(MAKE_DIRECTORY ${OSG_BUILD_DIR})
    ADD_SUBDIRECTORY(${OSG_SOURCE_DIR} ${OSG_BUILD_DIR})
ENDIF()

# Build example.
ADD_EXECUTABLE(osgemscripten osgemscripten.cpp)
# Make Emscripten generate ready-to-open HTML page.
SET(CMAKE_EXECUTABLE_SUFFIX ".html")
# Make Emscripten preload the resource.
SET_TARGET_PROPERTIES(osgemscripten PROPERTIES LINK_FLAGS "--preload-file box.osgt")
# Copy the resource to the build directory.
CONFIGURE_FILE(box.osgt box.osgt COPYONLY)
# Libraries must be linked in the specified order.
# Otherwise you may get unsatisified linker errors.
TARGET_LINK_LIBRARIES(
    osgemscripten
    osgViewer
    osgDB
    # osgDB plugins start.
    osgdb_osg
    osgdb_serializers_osg
    # osgDB plugins end.
    osgGA
    osgText
    osgUtil
    osg
    OpenThreads
)
