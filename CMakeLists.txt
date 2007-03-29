CMAKE_MINIMUM_REQUIRED(VERSION 2.4.6 FATAL_ERROR)

PROJECT(OpenSceneGraph)

# We have some custom .cmake scripts not in the official distribution.
# Maybe this can be used override existing behavior if needed?
SET(CMAKE_MODULE_PATH "${OpenSceneGraph_SOURCE_DIR}/CMakeModules;${CMAKE_MODULE_PATH}")

# Mainly for Windows as a convenience. This will find a directory in parallel with the
# OSG source that contains 3rd party headers and libraries.
# Use of relative paths in CMake is ill-advised, but don't know of any alternatives in this case
#SET(CMAKE_INCLUDE_PATH "${OpenSceneGraph_SOURCE_DIR}/../3rdParty/include;${CMAKE_INCLUDE_PATH}")
#SET(CMAKE_LIBRARY_PATH "${OpenSceneGraph_SOURCE_DIR}/../3rdParty/lib;${CMAKE_LIBRARY_PATH}")
IF(USING_OSG_OP_OT_TRIPLE_SET)
	SET(CMAKE_INCLUDE_PATH "${OpenSceneGraph_SOURCE_DIR}/../../3rdParty/include;${CMAKE_INCLUDE_PATH}")
	SET(CMAKE_LIBRARY_PATH "${OpenSceneGraph_SOURCE_DIR}/../../3rdParty/lib;${CMAKE_LIBRARY_PATH}")
ENDIF(USING_OSG_OP_OT_TRIPLE_SET)


# FIXME: The FindOpenThreads stuff below is not quite correct.
# The problem is that if we are building OpenSceneGraph by itself 
# (not part of the triple-set OT/OP/OSG source), then we need to hunt
# down the OpenThreads library on the system.
# But if we are building as part of the triple-set, then we want to 
# refer to the version in the triple set. But this gets harder because 
# FIND_LIBRARY will fail to pick the triple set version in this case 
# because the library is not yet built when running this CMake script.
#
# Maybe we need a global flag (set in the root CMakeLists.txt) 
# that tells us which scenario we are doing. 
# And in the triple set case, we skip this check.

IF(USING_OSG_OP_OT_TRIPLE_SET)
	# MESSAGE("OSG: Using TripleSet, ${OpenThreads_SOURCE_DIR}.")
	# So I think the fall-out is that all the OpenThreads variables
	# that have been set are still in play. So the include paths are still
	# visible, and the library is still set.
	# To keep the same code paths
	SET(OPENTHREADS_LIBRARY OpenThreads)
	SET(OPENTHREADS_INCLUDE_DIR ${OpenThreads_SOURCE_DIR}/include)

	# MESSAGE("Lib: ${OPENTHREADS_LIBRARY}")
	

ELSE(USING_OSG_OP_OT_TRIPLE_SET)
#	MESSAGE("OSG: Not using Triple Set")
	FIND_PACKAGE(OpenThreads REQUIRED)

ENDIF(USING_OSG_OP_OT_TRIPLE_SET)

# Okay, here's the problem: On some platforms, linking against OpenThreads
# is not enough and explicit linking to the underlying thread library 
# is also required (e.g. FreeBSD). But OpenThreads may be built with different
# backends (Pthreads, Sproc, Windows) so we don't know what the underlying 
# thread library is because some platforms support multiple backends (e.g. 
# IRIX supports Sproc and Pthreads). Linking all libraries won't work 
# because the libraries may be incompatible.
# So the current solution is to attempt best guess linking and exempt certain 
# cases. With IRIX, we're going to hope explicit linking to the underlying
# library is not necessary. We currently don't case for pthreads on Windows
# which might be an issue on things like Cygwin. This may need to be fixed.
FIND_PACKAGE(Threads)
IF(CMAKE_SYSTEM MATCHES IRIX)
	# Erase CMAKE_THREAD_LIBS_INIT and hope it works
	SET(CMAKE_THREAD_LIBS_INIT "" CACHE INTERNAL "")	
ENDIF(CMAKE_SYSTEM MATCHES IRIX)


# Find OpenGL
FIND_PACKAGE(OpenGL)

IF(APPLE)
	FIND_LIBRARY(CARBON_LIBRARY Carbon)
	FIND_LIBRARY(COCOA_LIBRARY Cocoa)
ENDIF(APPLE)

IF(UNIX)
	# Not sure what this will do on Cygwin and Msys
	# Also, remember OS X X11 is a user installed option so it may not exist.
	FIND_PACKAGE(X11)
	# Some Unicies need explicit linkage to the Math library or the build fails.
	FIND_LIBRARY(MATH_LIBRARY m)
ENDIF(UNIX)
	
# Make the headers visible to everything
INCLUDE_DIRECTORIES(
	${OpenSceneGraph_SOURCE_DIR}/include
	${OPENTHREADS_INCLUDE_DIR}
	${OPENGL_INCLUDE_DIR}
)
	
# Common global definitions
#ADD_DEFINITIONS(-D)
# Platform specific definitions

IF(WIN32)
	#needed for net plugin
	SET (OSG_SOCKET_LIBS wsock32.lib)
	# Both Cygwin and Msys need -DNOMINMAX ???
	IF(UNIX)
		ADD_DEFINITIONS(-DNOMINMAX)
	ENDIF(UNIX)
ENDIF(WIN32)

########################################################################################################
##### these were settings located in SetupCommon.cmake used in Luigi builds.... find out what are useful
########################################################################################################
#luigi#SET(CMAKE_VERBOSE_MAKEFILE TRUE)
#luigi#SET(CMAKE_SKIP_RPATH TRUE)
#luigi#SET(CMAKE_SKIP_RULE_DEPENDENCY TRUE)
#luigi#IF(UNIX)
#luigi#	LIST_CONTAINS(contains "g++" ${CMAKE_CXX_COMPILER_LIST})
#luigi#	IF (contains)
#luigi#		MESSAGE(${MY_MESSAGE_DEFAULT} "${CMAKE_CURRENT_LIST_FILE}:${CMAKE_CURRENT_LIST_LINE} setting  CMAKE_CXX_COMPILER to g++")
#luigi#		SET(CMAKE_CXX_COMPILER "g++")
#luigi#		SET(CMAKE_CXX_COMPILER_LOADED 2)
#luigi#		SET(CMAKE_CXX_COMPILER_WORKS 2)
#luigi#	ENDIF (contains)
#luigi#	SET(CMAKE_CXX_FLAGS_RELEASE "-O2")
#luigi#	SET(CMAKE_CXX_FLAGS_DEBUG "-ggdb -gstabs")
#luigi#ENDIF(UNIX)
########################################################################################################



################################################################################
# 3rd Party Dependency Stuff
IF(WIN32)
	INCLUDE(Find3rdPartyDependencies)
ENDIF(WIN32)

# Common to all platforms:
FIND_PACKAGE(FreeType)
FIND_PACKAGE(FLTK)
FIND_PACKAGE(GLUT)
FIND_PACKAGE(SDL)
FIND_PACKAGE(Inventor)
FIND_PACKAGE(Qt3)
FIND_PACKAGE(Qt4)

# Platform specific:
# (We can approach this one of two ways. We can try to FIND everything
# and simply check if we found the packages before actually building
# or we can hardcode the cases. The advantage of the former is that 
# packages that are installed on platforms that don't require them
# will still get built (presuming no compatibility issues). But this 
# also means modules that are redundant may get built. For example, 
# OS X doesn't need GIF, JPEG, PNG, TIFF, etc because it uses QuickTime.
# Also, it will clutter the CMake menu with "NOT_FOUND".
# The downside to the latter is that it is harder to build those
# potentially redundant modules.)

# Image readers/writers depend on 3rd party libraries except for OS X which
# can use Quicktime.
IF(NOT APPLE)
	FIND_PACKAGE(GIFLIB)
	FIND_PACKAGE(JPEG)
	FIND_PACKAGE(PNG)
	FIND_PACKAGE(TIFF)

	# QuickTime is required for OS X, but optional for Windows.
	IF(WIN32)
		FIND_PACKAGE(QuickTime)
	ENDIF(WIN32)
	
ELSE(NOT APPLE) 
	FIND_PACKAGE(QuickTime)
ENDIF(NOT APPLE)


################################################################################
# Create bin and lib directories if required

IF("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
   FILE(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/bin ${CMAKE_BINARY_DIR}/lib ${CMAKE_BINARY_DIR}/lib/osgPlugins)
ENDIF("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")


################################################################################
# Installation stuff

SET(CMAKE_DEBUG_POSTFIX  "d")

SET(LIB_POSTFIX "")
IF(UNIX AND NOT WIN32 AND NOT APPLE)
  IF(CMAKE_SIZEOF_VOID_P MATCHES "8")
  	SET(LIB_POSTFIX "64")
  ENDIF(CMAKE_SIZEOF_VOID_P MATCHES "8")
ENDIF(UNIX AND NOT WIN32 AND NOT APPLE)

#SET(OUTPUT_BINDIR ${PROJECT_BINARY_DIR}/bin/${CMAKE_SYSTEM_NAME})
SET(OUTPUT_BINDIR ${PROJECT_BINARY_DIR}/bin)
MAKE_DIRECTORY(${OUTPUT_BINDIR})
SET(EXECUTABLE_OUTPUT_PATH ${OUTPUT_BINDIR})

#SET(OUTPUT_LIBDIR ${PROJECT_BINARY_DIR}/lib/${CMAKE_SYSTEM_NAME})
SET(OUTPUT_LIBDIR ${PROJECT_BINARY_DIR}/lib)
MAKE_DIRECTORY(${OUTPUT_LIBDIR})
MAKE_DIRECTORY(${OUTPUT_LIBDIR}/osgPlugins)
SET(LIBRARY_OUTPUT_PATH ${OUTPUT_LIBDIR})



#SET(INSTALL_BINDIR OpenSceneGraph/bin)
#SET(INSTALL_INCDIR OpenSceneGraph/include)
#SET(INSTALL_LIBDIR OpenSceneGraph/lib)
#SET(INSTALL_DOCDIR OpenSceneGraph/doc)

################################################################################
# User Options

# Dynamic vs Static Linking
OPTION(DYNAMIC_OPENSCENEGRAPH "Set to ON to build OpenSceneGraph for dynamic linking.  Use OFF for static." ON)
IF   (DYNAMIC_OPENSCENEGRAPH)
	SET(OPENSCENEGRAPH_USER_DEFINED_DYNAMIC_OR_STATIC "SHARED")
ELSE (DYNAMIC_OPENSCENEGRAPH)
	SET(OPENSCENEGRAPH_USER_DEFINED_DYNAMIC_OR_STATIC "STATIC")
ENDIF(DYNAMIC_OPENSCENEGRAPH)

INCLUDE(OsgMacroUtils)
# OSG Core
ADD_SUBDIRECTORY(src)

# OSG Applications
OPTION(BUILD_OSG_APPLICATIONS "Enable to build OSG Applications (e.g. osgviewer)" ON)
IF   (BUILD_OSG_APPLICATIONS)
	ADD_SUBDIRECTORY(applications)
ENDIF(BUILD_OSG_APPLICATIONS)

# OSG Examples
OPTION(BUILD_OSG_EXAMPLES "Enable to build OSG Examples" OFF)
IF   (BUILD_OSG_EXAMPLES)
	ADD_SUBDIRECTORY(examples)
ENDIF(BUILD_OSG_EXAMPLES)



# For Doxygen
#FIXME: I haven't figured out what to do with OSG's multiple doxyfiles 
# and footer.
INCLUDE(${CMAKE_ROOT}/Modules/Documentation.cmake OPTIONAL)

# To build the documention, you will have to enable it
# and then do the equivalent of "make DoxygenDoc".
IF(BUILD_DOCUMENTATION)
	IF(DOT)
		SET(HAVE_DOT YES)
	ELSE(DOT)
		SET(HAVE_DOT NO)
	ENDIF(DOT)
	# This processes our Doxyfile.in and substitutes paths to generate
	# a final Doxyfile
	CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/doc/Doxyfiles/doxyfile.cmake
		${PROJECT_BINARY_DIR}/doc/doxyfile
	)
	# This creates a new target to build documentation.
	# It runs ${DOXYGEN} which is the full path and executable to
	# Doxygen on your system, set by the FindDoxygen.cmake module
	# (called by FindDocumentation.cmake).
	# It runs the final generated Doxyfile against it.
	# The DOT_PATH is substituted into the Doxyfile.
	ADD_CUSTOM_TARGET(DoxygenDoc ${DOXYGEN}
		${PROJECT_BINARY_DIR}/docs/doxyfile
	)
ENDIF(BUILD_DOCUMENTATION)

