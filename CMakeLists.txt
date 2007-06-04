IF(WIN32)
    CMAKE_MINIMUM_REQUIRED(VERSION 2.4.6 FATAL_ERROR)
ELSE(WIN32)
    IF(APPLE)
        CMAKE_MINIMUM_REQUIRED(VERSION 2.4.6 FATAL_ERROR)
        IF(${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 4 AND ${CMAKE_PATCH_VERSION} LESS 7)
            MESSAGE("Warning: A critical CMake bug exists in 2.4.6 and below. Trying to build Universal Binaries will result in a compile error that seems unrelated. Either avoid building Universal Binaries by changing the CMAKE_OSX_ARCHITECTURES field to list only your architecture, or upgrade to the current CVS version of CMake or a newer stable version if it exists.")
        ENDIF(${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 4 AND ${CMAKE_PATCH_VERSION} LESS 7)
    ELSE(APPLE)
        CMAKE_MINIMUM_REQUIRED(VERSION 2.4.0 FATAL_ERROR)
    ENDIF(APPLE)
ENDIF(WIN32)


PROJECT(OpenSceneGraph)

SET(OPENSCENEGRAPH_MAJOR_VERSION 1)
SET(OPENSCENEGRAPH_MINOR_VERSION 9)
SET(OPENSCENEGRAPH_PATCH_VERSION 7)
SET(OPENSCENEGRAPH_SOVERSION 9)

SET(OPENSCENEGRAPH_VERSION ${OPENSCENEGRAPH_MAJOR_VERSION}.${OPENSCENEGRAPH_MINOR_VERSION}.${OPENSCENEGRAPH_PATCH_VERSION})

SET(OSG_PLUGINS osgPlugins-${OPENSCENEGRAPH_VERSION})

# We want to build SONAMES shared librariess
SET(OPENSCENEGRAPH_SONAMES TRUE)
SET(OPENTHREADS_SONAMES TRUE)

SET(OpenThreads_SOURCE_DIR ${OpenSceneGraph_SOURCE_DIR})

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
    ${OPENGL_INCLUDE_DIR}
)
    
# Common global definitions
#ADD_DEFINITIONS(-D)
# Platform specific definitions

IF(WIN32)
    ADD_DEFINITIONS(-D_SCL_SECURE_NO_WARNINGS)
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
#luigi#    LIST_CONTAINS(contains "g++" ${CMAKE_CXX_COMPILER_LIST})
#luigi#    IF (contains)
#luigi#        MESSAGE(${MY_MESSAGE_DEFAULT} "${CMAKE_CURRENT_LIST_FILE}:${CMAKE_CURRENT_LIST_LINE} setting  CMAKE_CXX_COMPILER to g++")
#luigi#        SET(CMAKE_CXX_COMPILER "g++")
#luigi#        SET(CMAKE_CXX_COMPILER_LOADED 2)
#luigi#        SET(CMAKE_CXX_COMPILER_WORKS 2)
#luigi#    ENDIF (contains)
#luigi#    SET(CMAKE_CXX_FLAGS_RELEASE "-O2")
#luigi#    SET(CMAKE_CXX_FLAGS_DEBUG "-ggdb -gstabs")
#luigi#ENDIF(UNIX)
########################################################################################################

OPTION(OSG_USE_FLOAT_MATRIX "Set to ON to build OpenSceneGraph with float matrix instead of double." OFF)
MARK_AS_ADVANCED(OSG_USE_FLOAT_MATRIX)
IF(OSG_USE_FLOAT_MATRIX)        
    ADD_DEFINITIONS(-DOSG_USE_FLOAT_MATRIX)
ENDIF(OSG_USE_FLOAT_MATRIX)        

OPTION(OSG_USE_FLOAT_PLANE "Set to ON to build OpenSceneGraph with float matrix instead of double." OFF)
MARK_AS_ADVANCED(OSG_USE_FLOAT_PLANE)
IF(OSG_USE_FLOAT_PLANE)        
    ADD_DEFINITIONS(-DOSG_USE_FLOAT_PLANE)
ENDIF(OSG_USE_FLOAT_PLANE)        


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
FIND_PACKAGE(Jasper)
FIND_PACKAGE(COLLADA)
FIND_PACKAGE(Xine)
FIND_PACKAGE(OpenVRML)
FIND_PACKAGE(Performer)

# SET(DESIRED_QT_VERSION 3)
# FIND_PACKAGE(Qt)
FIND_PACKAGE(Qt3)
FIND_PACKAGE(Qt4)

SET(wxWidgets_USE_LIBS base core gl net)
FIND_PACKAGE(wxWidgets)

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
   FILE(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/bin ${CMAKE_BINARY_DIR}/lib ${CMAKE_BINARY_DIR}/lib/${OSG_PLUGINS})
ENDIF("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")


################################################################################
# Installation stuff

SET(CMAKE_DEBUG_POSTFIX  "d")

IF(UNIX AND NOT WIN32 AND NOT APPLE)
  IF(CMAKE_SIZEOF_VOID_P MATCHES "8")
      SET(LIB_POSTFIX "64" CACHE STRING "suffix for 32/64 dir placement")
      MARK_AS_ADVANCED(LIB_POSTFIX)
  ENDIF(CMAKE_SIZEOF_VOID_P MATCHES "8")
ENDIF(UNIX AND NOT WIN32 AND NOT APPLE)
IF(NOT DEFINED LIB_POSTFIX)
    SET(LIB_POSTFIX "")
ENDIF(NOT DEFINED LIB_POSTFIX)
    
#SET(OUTPUT_BINDIR ${PROJECT_BINARY_DIR}/bin/${CMAKE_SYSTEM_NAME})
SET(OUTPUT_BINDIR ${PROJECT_BINARY_DIR}/bin)
MAKE_DIRECTORY(${OUTPUT_BINDIR})
SET(EXECUTABLE_OUTPUT_PATH ${OUTPUT_BINDIR})

#SET(OUTPUT_LIBDIR ${PROJECT_BINARY_DIR}/lib/${CMAKE_SYSTEM_NAME})
SET(OUTPUT_LIBDIR ${PROJECT_BINARY_DIR}/lib)
MAKE_DIRECTORY(${OUTPUT_LIBDIR})
MAKE_DIRECTORY(${OUTPUT_LIBDIR}/${OSG_PLUGINS})
SET(LIBRARY_OUTPUT_PATH ${OUTPUT_LIBDIR})



#SET(INSTALL_BINDIR OpenSceneGraph/bin)
#SET(INSTALL_INCDIR OpenSceneGraph/include)
#SET(INSTALL_LIBDIR OpenSceneGraph/lib)
#SET(INSTALL_DOCDIR OpenSceneGraph/doc)

################################################################################
# User Options


# Expose CMAKE_INCLUDE_PATH and CMAKE_LIBARY_PATH to the GUI so users
# may set these values without needing to manipulate the environment.
SET(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} CACHE STRING "You may add additional search paths here. Use ; to separate multiple paths.")
SET(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} CACHE STRING "You may add additional search paths here. Use ; to separate multiple paths.")
# We are proposing that a new variable called CMAKE_PREFIX_PATH be introduced
# to CMake to compliment CMAKE_INCLUDE_PATH and CMAKE_LIBRARY_PATH.
# A formal feature request has been submited to CMake, Bug #4947.
# It is intended for those users who have common prefixes for their INCLUDE
# and LIBRARY locations. So if users have headers in /usr/local/include
# and libraries in /usr/local/lib, the common prefix is /usr/local.
# It should also cover the case where headers and libraries are
# in the same directory.
# Our proposal expects that FIND_* commands will automatically search for
# CMAKE_PREFIX_PATH right after CMAKE_INCLUDE_PATH or CMAKE_LIBRARY_PATH.
# Obviously, since CMake does not currently support this, we must write
# our Find*.cmake modules to explicitly support this. Otherwise, this variable
# will have no impact.
# This is unofficial so this may be removed or changed at anytime.
SET(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} CACHE STRING "(EXPERIMENTAL) You may add additional search paths here. Use ; to separate multiple paths.")


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

# This is for an advanced option to give aggressive warnings 
# under different compilers. If yours is not implemented, this option
# will not be made available.
IF(CMAKE_COMPILER_IS_GNUCXX)
    # To be complete, we might also do GNUCC flags, 
    # but everything here is C++ code.
    # -Wshadow and -Woverloaded-virtual are also interesting flags, but OSG
    # returns too many hits.
    # FYI, if we do implement GNUCC, then -Wmissing-prototypes in another 
    # interesting C-specific flag.
    # Also, there is a bug in gcc 4.0. Under C++, -pedantic will create
    # errors instead of warnings for certain issues, including superfluous
    # semicolons and commas, and the use of long long. -fpermissive seems
    # to be the workaround. 
    SET(OSG_AGGRESSIVE_WARNING_FLAGS "-Wall -Wparentheses -Wformat=2 -Wno-long-long -Wno-import -pedantic -Wnewline-eof -Wreturn-type -Wmissing-braces -Wunknown-pragmas -Wunused -fpermissive")
ELSE(CMAKE_COMPILER_IS_GNUCXX)
    IF(MSVC)
        # FIXME: What are good aggressive warning flags for Visual Studio?
        # And do we need to further subcase this for different versions of VS?
        # CMake variables: MSVC60, MSVC70, MSVC71, MSVC80, CMAKE_COMPILER_2005
        SET(OSG_AGGRESSIVE_WARNING_FLAGS "/Wall /W4")
    ELSE(MSVC)
        # CMake lacks an elseif, so other non-gcc, non-VS compilers need
        # to be listed below. If unhandled, OSG_AGGRESSIVE_WARNING_FLAGS should 
        # remain unset.
    ENDIF(MSVC)
ENDIF(CMAKE_COMPILER_IS_GNUCXX)

# This part is for the CMake menu option to toggle the warnings on/off.
# This will only be made available if we set values for OSG_AGGRESSIVE_WARNING_FLAGS.
IF(OSG_AGGRESSIVE_WARNING_FLAGS)
    OPTION(OSG_USE_AGGRESSIVE_WARNINGS "Enable to activate aggressive warnings" OFF)
    MARK_AS_ADVANCED(OSG_USE_AGGRESSIVE_WARNINGS)

    IF(OSG_USE_AGGRESSIVE_WARNINGS)
        IF(NOT "${OLD_CMAKE_CXX_FLAGS_WAS_SET}")
            SET(OLD_CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" CACHE INTERNAL "Old CXX flags")
            SET(OLD_CMAKE_CXX_FLAGS_WAS_SET 1 CACHE INTERNAL "Old CXX flags was set")
            SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OSG_AGGRESSIVE_WARNING_FLAGS}" CACHE STRING "Flags used by the compiler during all build types." FORCE)
        ENDIF(NOT "${OLD_CMAKE_CXX_FLAGS_WAS_SET}")
    ELSE(OSG_USE_AGGRESSIVE_WARNINGS)
        # FIXME: This will lose any changes made after OLD_CMAKE_CXX_FLAGS was 
        # set. The better way would be to parse the string and remove each
        # option explicitly.
        IF("${OLD_CMAKE_CXX_FLAGS_WAS_SET}")
            SET(CMAKE_CXX_FLAGS "${OLD_CMAKE_CXX_FLAGS}" CACHE STRING "Flags used by the compiler during all build types." FORCE)
            SET(OLD_CMAKE_CXX_FLAGS_WAS_SET 0 CACHE INTERNAL "Old CXX flags was set")
        ENDIF("${OLD_CMAKE_CXX_FLAGS_WAS_SET}")
    ENDIF(OSG_USE_AGGRESSIVE_WARNINGS)
ENDIF(OSG_AGGRESSIVE_WARNING_FLAGS)

# Set defaults for Universal Binaries. We want 32-bit Intel/PPC on 10.4 
# and 32/64-bit Intel/PPC on >= 10.5. Anything <= 10.3 doesn't support.
IF(APPLE)
    # These are just defaults/recommendations, but how we want to build
    # out of the box. But the user needs to be able to change these options.
    # So we must only set the values the first time CMake is run, or we 
    # will overwrite any changes the user sets.
    # FORCE is used because the options are not reflected in the UI otherwise.
    # Seems like a good place to add version specific compiler flags too.
    IF(NOT OSG_CONFIG_HAS_BEEN_RUN_BEFORE)
        # This is really fragile, but CMake doesn't provide the OS system 
        # version information we need. (Darwin versions can be changed 
        # independently of OS X versions.)
        # It does look like CMake handles the CMAKE_OSX_SYSROOT automatically.
        IF(EXISTS /Developer/SDKs/10.5.sdk)
            SET(CMAKE_OSX_ARCHITECTURES "ppc;i386;ppc64;x86_64" CACHE STRING "Build architectures for OSX" FORCE)
            SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mmacosx-version-min=10.5 -ftree-vectorize -fvisibility-inlines-hidden" CACHE STRING "Flags used by the compiler during all build types." FORCE)
        ELSE(EXISTS /Developer/SDKs/10.5.sdk)
            IF(EXISTS /Developer/SDKs/MacOSX10.4u.sdk)
                SET(CMAKE_OSX_ARCHITECTURES "ppc;i386" CACHE STRING "Build architectures for OSX" FORCE)
                SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mmacosx-version-min=10.4 -ftree-vectorize -fvisibility-inlines-hidden" CACHE STRING "Flags used by the compiler during all build types." FORCE)
            ELSE(EXISTS /Developer/SDKs/MacOSX10.4u.sdk)
                # No Universal Binary support
                # Should break down further to set the -mmacosx-version-min,
                # but the SDK detection is too unreliable here.
            ENDIF(EXISTS /Developer/SDKs/MacOSX10.4u.sdk)
        ENDIF(EXISTS /Developer/SDKs/10.5.sdk)
    ENDIF(NOT OSG_CONFIG_HAS_BEEN_RUN_BEFORE)
ENDIF(APPLE)


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

# This needs to be run very last so other parts of the scripts can take
# advantage of this.
IF(NOT OSG_CONFIG_HAS_BEEN_RUN_BEFORE)
    SET(OSG_CONFIG_HAS_BEEN_RUN_BEFORE 1 CACHE INTERNAL "Flag to track whether this is the first time running CMake or if CMake has been configured before")
ENDIF(NOT OSG_CONFIG_HAS_BEEN_RUN_BEFORE)


#-----------------------------------------------------------------------------
### uninstall target
#-----------------------------------------------------------------------------
CONFIGURE_FILE(
  "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)
ADD_CUSTOM_TARGET(uninstall
  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

