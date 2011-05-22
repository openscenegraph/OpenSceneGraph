set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS TRUE)

IF(WIN32)
    CMAKE_MINIMUM_REQUIRED(VERSION 2.4.6 FATAL_ERROR)
ELSE(WIN32)
    IF(APPLE)
        CMAKE_MINIMUM_REQUIRED(VERSION 2.6.0 FATAL_ERROR)
    ELSE(APPLE)
        CMAKE_MINIMUM_REQUIRED(VERSION 2.4.0 FATAL_ERROR)
    ENDIF(APPLE)
ENDIF(WIN32)

if(COMMAND cmake_policy)
    # Works around warnings libraries linked against that don't
    # have absolute paths (e.g. -lpthreads)
    cmake_policy(SET CMP0003 NEW)

    # Works around warnings about escaped quotes in ADD_DEFINITIONS
    # statements.
    cmake_policy(SET CMP0005 NEW)

    # cmake-2.6.1 introduces policy cmp0008 decide how to treat full path libraries that do not appear to be valid library file names
    # quote from cvslog "Such libraries worked by accident in the VS IDE and Xcode generators in CMake 2.4 and below."
    if(${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} GREATER 4 AND ${CMAKE_PATCH_VERSION} GREATER 0)
        cmake_policy(SET CMP0008 OLD)
    endif(${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} GREATER 4 AND ${CMAKE_PATCH_VERSION} GREATER 0)
endif(COMMAND cmake_policy)

PROJECT(OpenSceneGraph)

SET(OPENSCENEGRAPH_MAJOR_VERSION 2)
SET(OPENSCENEGRAPH_MINOR_VERSION 8)
SET(OPENSCENEGRAPH_PATCH_VERSION 5)
SET(OPENSCENEGRAPH_SOVERSION 74)

# set to 0 when not a release candidate, non zero means that any generated 
# svn tags will be treated as release candidates of given number
SET(OPENSCENEGRAPH_RELEASE_CANDIDATE 0)

SET(OPENSCENEGRAPH_VERSION ${OPENSCENEGRAPH_MAJOR_VERSION}.${OPENSCENEGRAPH_MINOR_VERSION}.${OPENSCENEGRAPH_PATCH_VERSION})

SET(OSG_PLUGINS osgPlugins-${OPENSCENEGRAPH_VERSION})

SET(OSG_PLUGIN_PREFIX "")

IF (CYGWIN)
    SET(OSG_PLUGIN_PREFIX "cygwin_")
ENDIF(CYGWIN)

IF(MINGW)
    SET(OSG_PLUGIN_PREFIX "mingw_")
ENDIF(MINGW)


# We want to build SONAMES shared librariess
SET(OPENSCENEGRAPH_SONAMES TRUE)
SET(OPENTHREADS_SONAMES TRUE)

SET(OpenThreads_SOURCE_DIR ${OpenSceneGraph_SOURCE_DIR})

# We have some custom .cmake scripts not in the official distribution.
# Maybe this can be used override existing behavior if needed?
SET(CMAKE_MODULE_PATH "${OpenSceneGraph_SOURCE_DIR}/CMakeModules;${CMAKE_MODULE_PATH}")


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

OPTION(OSG_MAINTAINER "Enable OpenSceneGraph maintainer build methods, such as making svn branches, tags, updating ChangeLog." OFF)
IF (OSG_MAINTAINER)

    #SET(OPENSCENEGRAPH_SVN "trunk")
    SET(OPENSCENEGRAPH_SVN "branches")

    SET(OPENSCENEGRAPH_BRANCH OpenSceneGraph-${OPENSCENEGRAPH_MAJOR_VERSION}.${OPENSCENEGRAPH_MINOR_VERSION})

    #
    # Provide target for tagging a release
    #
    SET(SVNCOMMAND svn)
    SET(SVNTRUNKDIR     http://www.openscenegraph.org/svn/osg/OpenSceneGraph/trunk)
    SET(SVNTAGDIR       http://www.openscenegraph.org/svn/osg/OpenSceneGraph/tags)
    SET(SVNBRANCHDIR    http://www.openscenegraph.org/svn/osg/OpenSceneGraph/branches)

    IF   (OPENSCENEGRAPH_SVN STREQUAL "trunk")
        SET(SVNSOURCEDIR ${SVNTRUNKDIR})
    ELSE (OPENSCENEGRAPH_SVN STREQUAL "trunk")
        SET(SVNSOURCEDIR ${SVNBRANCHDIR}/${OPENSCENEGRAPH_BRANCH})
    ENDIF(OPENSCENEGRAPH_SVN STREQUAL "trunk")


    IF   (OPENSCENEGRAPH_RELEASE_CANDIDATE EQUAL 0)
        SET(RELEASE_NAME OpenSceneGraph-${OPENSCENEGRAPH_VERSION})
    ELSE (OPENSCENEGRAPH_RELEASE_CANDIDATE EQUAL 0)
        SET(RELEASE_NAME OpenSceneGraph-${OPENSCENEGRAPH_VERSION}-rc${OPENSCENEGRAPH_RELEASE_CANDIDATE})
    ENDIF(OPENSCENEGRAPH_RELEASE_CANDIDATE EQUAL 0)


    ADD_CUSTOM_TARGET(tag-test
        COMMAND echo ${SVNCOMMAND} copy ${SVNSOURCEDIR} ${SVNTAGDIR}/${RELEASE_NAME} -m "Release ${RELEASE_NAME}"
    )

    ADD_CUSTOM_TARGET(tag-run
        COMMAND ${SVNCOMMAND} copy ${SVNSOURCEDIR} ${SVNTAGDIR}/${RELEASE_NAME} -m "Release ${RELEASE_NAME}"
    )

    ADD_CUSTOM_TARGET(branch-test
        COMMAND echo ${SVNCOMMAND} copy ${SVNSOURCEDIR} ${SVNBRANCHDIR}/${OPENSCENEGRAPH_BRANCH} -m "Branch ${OPENSCENEGRAPH_BRANCH}"
    )

    ADD_CUSTOM_TARGET(branch-run
        COMMAND ${SVNCOMMAND} copy ${SVNSOURCEDIR} ${SVNBRANCHDIR}/${OPENSCENEGRAPH_BRANCH} -m "Branch ${OPENSCENEGRAPH_BRANCH}"
    )

    #
    # Provide target for generating ChangeLog
    #
    SET(GENERATELOGS svn2cl)

    ADD_CUSTOM_TARGET(ChangeLog
        COMMAND ${SVNCOMMAND} update
        COMMAND ${GENERATELOGS} ${SVNSOURCEDIR}
    )
    
ENDIF(OSG_MAINTAINER)


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

    FIND_LIBRARY(DL_LIBRARY dl)
    IF(NOT DL_LIBRARY)
        SET(DL_LIBRARY "") # change from NOTFOUND to empty when passed to linker
    ENDIF()

ENDIF(UNIX)

# Make the headers visible to everything
IF(NOT ${PROJECT_BINARY_DIR} EQUAL ${PROJECT_SOURCE_DIR})
   INCLUDE_DIRECTORIES(${PROJECT_BINARY_DIR}/include)
ENDIF(NOT ${PROJECT_BINARY_DIR} EQUAL ${PROJECT_SOURCE_DIR})

INCLUDE_DIRECTORIES(
    ${OpenSceneGraph_SOURCE_DIR}/include
    ${OPENGL_INCLUDE_DIR}
)

# Common global definitions
#ADD_DEFINITIONS(-D)
# Platform specific definitions


IF(WIN32)

    IF(MSVC)
        # This option is to enable the /MP switch for Visual Studio 2005 and above compilers
        OPTION(WIN32_USE_MP "Set to ON to build OpenSceneGraph with the /MP option (Visual Studio 2005 and above)." OFF)
        MARK_AS_ADVANCED(WIN32_USE_MP)
        IF(WIN32_USE_MP)
            SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
        ENDIF(WIN32_USE_MP)

        # turn off various warnings
        # foreach(warning 4244 4251 4267 4275 4290 4786 4305 4996)
        #     SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd${warning}")
        # endforeach(warning)

        # More MSVC specific compilation flags
        ADD_DEFINITIONS(-D_SCL_SECURE_NO_WARNINGS)
        ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE)
    ENDIF(MSVC)

    #needed for net plugin
    SET (OSG_SOCKET_LIBS wsock32)
    # Both Cygwin and Msys need -DNOMINMAX ???
    IF(UNIX)
        ADD_DEFINITIONS(-DNOMINMAX)
    ENDIF(UNIX)
########################################################################################################
# the following options are MSVC specific,
# the first OSG_MSVC_VERSIONED_DLL activate a custom build-time layout that should allow to run examples and application
# fron bin folder without requiring installation step.
# it also prepend "osg${OPENSCENEGRAPH_SOVERSION}-" to only .dll files, leaving .lib files untouched in lib
# it also use a hack to get rid of Debug and Release folder in MSVC projects
# all the .dll and .pdb are in bin and all the .lib and .exp are in lib
#
# the second option disable incremental linking in debug build , that is enabled by default by CMake
##########################################################################################################

    IF(MSVC)
        IF(${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 4 AND ${CMAKE_PATCH_VERSION} LESS 7)
            MESSAGE("Warning:  disabling versioned options 2.4.6 exibits inconsintencies in .pdb naming, at least under MSVC, suggested upgrading at least to 2.4.7")
            SET(OSG_MSVC_VERSIONED_DLL OFF)
            SET(OSG_MSVC_DEBUG_INCREMENTAL_LINK ON)
        ELSE(${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 4 AND ${CMAKE_PATCH_VERSION} LESS 7)
            OPTION(OSG_MSVC_VERSIONED_DLL "Set to ON to build OpenSceneGraph with versioned dll names" ON)
            MARK_AS_ADVANCED(OSG_MSVC_VERSIONED_DLL)
            OPTION(OSG_MSVC_DEBUG_INCREMENTAL_LINK "Set to OFF to build OpenSceneGraph without incremental linking in debug (release is off by default)" ON)
            MARK_AS_ADVANCED(OSG_MSVC_DEBUG_INCREMENTAL_LINK)
            IF(NOT OSG_MSVC_DEBUG_INCREMENTAL_LINK)
                SET(CMAKE_MODULE_LINKER_FLAGS_DEBUG "/debug /INCREMENTAL:NO")
                SET(CMAKE_SHARED_LINKER_FLAGS_DEBUG "/debug /INCREMENTAL:NO")
                SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "/debug /INCREMENTAL:NO")
            ENDIF(NOT OSG_MSVC_DEBUG_INCREMENTAL_LINK)
        ENDIF(${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 4 AND ${CMAKE_PATCH_VERSION} LESS 7)
    ENDIF(MSVC)
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

OPTION(OSG_NOTIFY_DISABLED "Set to ON to build OpenSceneGraph with the notify() disabled." OFF)

OPTION(OSG_USE_FLOAT_MATRIX "Set to ON to build OpenSceneGraph with float Matrix instead of double." OFF)
MARK_AS_ADVANCED(OSG_USE_FLOAT_MATRIX)

OPTION(OSG_USE_FLOAT_PLANE "Set to ON to build OpenSceneGraph with float Plane instead of double." OFF)
MARK_AS_ADVANCED(OSG_USE_FLOAT_PLANE)

OPTION(OSG_USE_FLOAT_BOUNDINGSPHERE "Set to ON to build OpenSceneGraph with float BoundingSphere instead of double." ON)
MARK_AS_ADVANCED(OSG_USE_FLOAT_BOUNDINGSPHERE)

OPTION(OSG_USE_FLOAT_BOUNDINGBOX "Set to ON to build OpenSceneGraph with float BoundingBox instead of double." ON)
MARK_AS_ADVANCED(OSG_USE_FLOAT_BOUNDINGBOX)

OPTION(OSG_USE_UTF8_FILENAME "Set to ON to use a UTF8 locale for filenames instead of the default locale." OFF)
MARK_AS_ADVANCED(OSG_USE_UTF8_FILENAME)

OPTION(OSG_DISABLE_MSVC_WARNINGS "Set to OFF to not disable MSVC warnings generated by OSG headers." ON)
MARK_AS_ADVANCED(OSG_DISABLE_MSVC_WARNINGS)

OPTION(OSG_USE_REF_PTR_IMPLICIT_OUTPUT_CONVERSION "Set to ON to use the ref_ptr<> T* operator() output conversion. " ON)

OPTION(OSG_CPP_EXCEPTIONS_AVAILABLE "Set to OFF to disable compile of OSG components that use C++ exceptions." ON)

################################################################################
# Set Config file

SET(OPENSCENEGRAPH_CONFIG_HEADER "${PROJECT_BINARY_DIR}/include/osg/Config")
CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/src/osg/Config.in"
               "${OPENSCENEGRAPH_CONFIG_HEADER}")
# INSTALL_FILES(/include/osg/ FILES "${OPENSCENEGRAPH_CONFIG_HEADER}")


################################################################################
# Optional build components

# OSG Applications
OPTION(BUILD_OSG_APPLICATIONS "Enable to build OSG Applications (e.g. osgviewer)" ON)

# OSG Examples
OPTION(BUILD_OSG_EXAMPLES "Enable to build OSG Examples" OFF)

################################################################################
# 3rd Party Dependency Stuff
IF(WIN32)
    INCLUDE(Find3rdPartyDependencies)
ENDIF(WIN32)

# Common to all platforms:
FIND_PACKAGE(FreeType)
FIND_PACKAGE(Inventor)
FIND_PACKAGE(Jasper)
FIND_PACKAGE(OpenEXR)
FIND_PACKAGE(COLLADA)
FIND_PACKAGE(FBX)
FIND_PACKAGE(ZLIB)
FIND_PACKAGE(Xine)
FIND_PACKAGE(OpenVRML)
FIND_PACKAGE(Performer)
FIND_PACKAGE(GDAL)
FIND_PACKAGE(CURL)
FIND_PACKAGE(ITK)
FIND_PACKAGE(LibVNCServer)
FIND_PACKAGE(OurDCMTK)
FIND_PACKAGE(OpenAL)
FIND_PACKAGE(XUL)
FIND_PACKAGE(FFmpeg)

# Include macro utilities here
INCLUDE(OsgMacroUtils)

#use pkg-config to find various modues
INCLUDE(FindPkgConfig OPTIONAL)

IF(PKG_CONFIG_FOUND)

    INCLUDE(FindPkgConfig)

    PKG_CHECK_MODULES(GTK gtk+-2.0)

    IF(WIN32)
        PKG_CHECK_MODULES(GTKGL gtkglext-win32-1.0)
    ELSE(WIN32)
        PKG_CHECK_MODULES(GTKGL gtkglext-x11-1.0)
    ENDIF(WIN32)

    PKG_CHECK_MODULES(RSVG librsvg-2.0)
    PKG_CHECK_MODULES(CAIRO cairo)
    PKG_CHECK_MODULES(POPPLER poppler-glib)

ENDIF(PKG_CONFIG_FOUND)

#optional example related dependencies
IF   (BUILD_OSG_EXAMPLES)


    FIND_PACKAGE(FLTK)
    FIND_PACKAGE(GLUT)
    FIND_PACKAGE(SDL)
    FIND_PACKAGE(FOX)

    SET(wxWidgets_USE_LIBS base core gl net)
    FIND_PACKAGE(wxWidgets)

    # To select a specific version of QT define DESIRED_QT_VERSION
    # via cmake -DDESIRED_QT_VERSION=4
    IF  (DESIRED_QT_VERSION)

        IF  (DESIRED_QT_VERSION MATCHES 4)
          FIND_PACKAGE(Qt4)
        ELSE(DESIRED_QT_VERSION MATCHES 4)
          FIND_PACKAGE(Qt3)
        ENDIF(DESIRED_QT_VERSION MATCHES 4)

    ELSE(DESIRED_QT_VERSION)

        FIND_PACKAGE(Qt4)

        IF  (NOT QT4_FOUND)
            FIND_PACKAGE(Qt3)
        ENDIF(NOT QT4_FOUND)

    ENDIF(DESIRED_QT_VERSION)

ENDIF(BUILD_OSG_EXAMPLES)






#
# Test to determine if we want the "tripledot" form of the GLU tesselator callback.
#
IF(NOT DEFAULT_GLU_TESS_CALLBACK_TRIPLEDOT)
    IF(WIN32 OR CMAKE_SYSTEM_NAME MATCHES "Linux")

        # Skip the compile check for platforms that never need the variable
        # form.
        SET(DEFAULT_GLU_TESS_CALLBACK_TRIPLEDOT false)

    ELSE(WIN32 OR CMAKE_SYSTEM_NAME MATCHES "Linux")

        # For other platforms perform the check
        INCLUDE(CheckCXXSourceCompiles)
        SET(CMAKE_REQUIRED_DEFINITIONS -DGLU_TESS_CALLBACK_TRIPLEDOT)
        SET(CMAKE_REQUIRED_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/include ${GLUT_INCLUDE_DIR} ${GL_INCLUDE_DIR})
        SET(CMAKE_REQUIRED_LIBRARIES ${GLUT_LIBRARY} ${GL_LIBRARY})
        CHECK_CXX_SOURCE_COMPILES(
            "#include <osg/GL>
            #include <osg/GLU>
            static void testcb(GLvoid *, void*) { }
            int main() {
               GLUtesselator *t = gluNewTess();
               gluTessCallback(t, GLU_TESS_VERTEX_DATA, (GLU_TESS_CALLBACK) testcb);
               return 0;
            }"
            GLU_Tesselator_Needs_Variable_Parameter_Callback_Convention_Failure_Means_No)
        SET(DEFAULT_GLU_TESS_CALLBACK_TRIPLEDOT
            ${GLU_Tesselator_Needs_Variable_Parameter_Callback_Convention_Failure_Means_No})

    ENDIF(WIN32 OR CMAKE_SYSTEM_NAME MATCHES "Linux")
ENDIF(NOT DEFAULT_GLU_TESS_CALLBACK_TRIPLEDOT)

OPTION(OSG_GLU_TESS_CALLBACK_TRIPLEDOT "Set to ON to build with variable parameter (...) version of GLU tesselator callback" ${DEFAULT_GLU_TESS_CALLBACK_TRIPLEDOT})
IF(OSG_GLU_TESS_CALLBACK_TRIPLEDOT)
    ADD_DEFINITIONS(-DGLU_TESS_CALLBACK_TRIPLEDOT)
ENDIF(OSG_GLU_TESS_CALLBACK_TRIPLEDOT)

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

SET(CMAKE_DEBUG_POSTFIX "d" CACHE STRING "add a postfix, usually d on windows")
ADD_DEFINITIONS(-DOSG_DEBUG_POSTFIX=${CMAKE_DEBUG_POSTFIX})

IF(UNIX AND NOT WIN32)
  IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
    ADD_DEFINITIONS("-D_DEBUG")
  ENDIF(CMAKE_BUILD_TYPE STREQUAL "Debug")
ENDIF(UNIX AND NOT WIN32)

IF(CYGWIN)
  IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
    ADD_DEFINITIONS("-D_DEBUG")
  ENDIF(CMAKE_BUILD_TYPE STREQUAL "Debug")
ENDIF(CYGWIN)

IF(UNIX AND NOT WIN32 AND NOT APPLE)
  IF(CMAKE_SIZEOF_VOID_P MATCHES "8")
      SET(LIB_POSTFIX "64" CACHE STRING "suffix for 32/64 dir placement")
      MARK_AS_ADVANCED(LIB_POSTFIX)
  ENDIF(CMAKE_SIZEOF_VOID_P MATCHES "8")
ENDIF(UNIX AND NOT WIN32 AND NOT APPLE)
IF(NOT DEFINED LIB_POSTFIX)
    SET(LIB_POSTFIX "")
ENDIF(NOT DEFINED LIB_POSTFIX)

# Here we apparantly do some funky stuff with making the bin/ and lib/
# folders which is probably needed to work around a very old CMake bug?

#SET(OUTPUT_BINDIR ${PROJECT_BINARY_DIR}/bin/${CMAKE_SYSTEM_NAME})
SET(OUTPUT_BINDIR ${PROJECT_BINARY_DIR}/bin)
MAKE_DIRECTORY(${OUTPUT_BINDIR})
IF(MSVC AND NOT MSVC_IDE)
    MAKE_DIRECTORY(${OUTPUT_BINDIR}/${OSG_PLUGINS})
ENDIF(MSVC AND NOT MSVC_IDE)

#SET(OUTPUT_LIBDIR ${PROJECT_BINARY_DIR}/lib/${CMAKE_SYSTEM_NAME})
SET(OUTPUT_LIBDIR ${PROJECT_BINARY_DIR}/lib)
MAKE_DIRECTORY(${OUTPUT_LIBDIR})
IF(NOT MSVC)
    MAKE_DIRECTORY(${OUTPUT_LIBDIR}/${OSG_PLUGINS})
ENDIF(NOT MSVC)

# On CMake 2.4.x use EXECUTABLE_OUTPUT_PATH and LIBRARY_OUTPUT_PATH and later
# we work around the DLL placement by use of the PREFIX target property hack
#
# On CMake 2.6.x use the newly minted CMAKE_LIBRARY_OUTPUT_DIRECTORY,
# CMAKE_ARCHIVE_OUTPUT_DIRECTORY & CMAKE_RUNTIME_OUTPUT_DIRECTORY
#
# CMake >= 2.8.1 changed the output directory algorithm (See doc).
# Here we also set per-configuration directories (CMAKE_*_OUTPUT_DIRECTORY_<CONFIG>), or else binaries are generated in /bin/Debug and /bin/Release, etc. with MSVC and Xcode.
# (Doc reads "multi-configuration generators (VS, Xcode) do NOT append a per-configuration subdirectory to the specified directory").
# The workaround for 2.6.x (adding "../" as an output prefix for each target) seem to have no effect in >=2.8.1, so there is no need to change this.
IF(CMAKE_MAJOR_VERSION EQUAL 2 AND CMAKE_MINOR_VERSION LESS 5)
    # If CMake < 2.6.0
    SET(EXECUTABLE_OUTPUT_PATH ${OUTPUT_BINDIR})
    SET(LIBRARY_OUTPUT_PATH    ${OUTPUT_LIBDIR})
ELSE()
    # If CMake >= 2.6.0
    SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_LIBDIR})
    SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_BINDIR})
    IF(WIN32)
        SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_BINDIR})
    ELSE(WIN32)
        SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_LIBDIR})
    ENDIF(WIN32)

    # Testing CMAKE_VERSION is possible in >= 2.6.4 only
    BUILDER_VERSION_GREATER(2 8 0)
    IF(VALID_BUILDER_VERSION)  # If CMake >= 2.8.1
        FOREACH(CONF ${CMAKE_CONFIGURATION_TYPES})        # For each configuration (Debug, Release, MinSizeRel... and/or anything the user chooses)
            STRING(TOUPPER "${CONF}" CONF)                # Go uppercase (DEBUG, RELEASE...)
            SET("CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CONF}" "${OUTPUT_LIBDIR}")
            SET("CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONF}" "${OUTPUT_BINDIR}")
            IF(WIN32)
                SET("CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CONF}" "${OUTPUT_BINDIR}")
            ELSE()
                SET("CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CONF}" "${OUTPUT_LIBDIR}")
            ENDIF()
        ENDFOREACH()
    ENDIF(VALID_BUILDER_VERSION)
ENDIF()

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
    SET(OSG_AGGRESSIVE_WARNING_FLAGS -Wall -Wparentheses -Wno-long-long -Wno-import -pedantic -Wreturn-type -Wmissing-braces -Wunknown-pragmas -Wunused -fpermissive)

    # Previous included -Wformat=2 in OSG_AGGRESSIVE_WARNING_FLAGS but had to remove it due to standard library errors


ELSE(CMAKE_COMPILER_IS_GNUCXX)
    IF(MSVC)
        # FIXME: What are good aggressive warning flags for Visual Studio?
        # And do we need to further subcase this for different versions of VS?
        # CMake variables: MSVC60, MSVC70, MSVC71, MSVC80, CMAKE_COMPILER_2005
        SET(OSG_AGGRESSIVE_WARNING_FLAGS /W4 /wd4706 /wd4127 /wd4100)


    ELSE(MSVC)
        # CMake lacks an elseif, so other non-gcc, non-VS compilers need
        # to be listed below. If unhandled, OSG_AGGRESSIVE_WARNING_FLAGS should
        # remain unset.
    ENDIF(MSVC)
ENDIF(CMAKE_COMPILER_IS_GNUCXX)

# This part is for the CMake menu option to toggle the warnings on/off.
# This will only be made available if we set values for OSG_AGGRESSIVE_WARNING_FLAGS.
IF(OSG_AGGRESSIVE_WARNING_FLAGS)

    IF (APPLE)
        SET(DEFAULT_USE_AGGRESSIVE_WARNINGS OFF)
    ELSE(APPLE)
        SET(DEFAULT_USE_AGGRESSIVE_WARNINGS ON)
    ENDIF(APPLE)

    OPTION(OSG_USE_AGGRESSIVE_WARNINGS "Enable to activate aggressive warnings" ${DEFAULT_USE_AGGRESSIVE_WARNINGS})
    MARK_AS_ADVANCED(OSG_USE_AGGRESSIVE_WARNINGS)

    IF(OSG_USE_AGGRESSIVE_WARNINGS)
        # Add flags defined by OSG_AGGRESSIVE_WARNING_FLAGS if they aren't already there
        FOREACH(flag ${OSG_AGGRESSIVE_WARNING_FLAGS})
            IF(NOT CMAKE_CXX_FLAGS MATCHES "${flag}")
                SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}")
            ENDIF(NOT CMAKE_CXX_FLAGS MATCHES "${flag}")
        ENDFOREACH(flag)
    ELSE(OSG_USE_AGGRESSIVE_WARNINGS)
        # Remove all flags considered aggresive
        FOREACH(flag ${OSG_AGGRESSIVE_WARNING_FLAGS})
            STRING(REGEX REPLACE "${flag}" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
        ENDFOREACH(flag)
    ENDIF(OSG_USE_AGGRESSIVE_WARNINGS)
ENDIF(OSG_AGGRESSIVE_WARNING_FLAGS)


# Dynamic vs Static Linking
OPTION(DYNAMIC_OPENSCENEGRAPH "Set to ON to build OpenSceneGraph for dynamic linking.  Use OFF for static." ON)
IF   (DYNAMIC_OPENSCENEGRAPH)
    SET(OPENSCENEGRAPH_USER_DEFINED_DYNAMIC_OR_STATIC "SHARED")
ELSE (DYNAMIC_OPENSCENEGRAPH)
    SET(OPENSCENEGRAPH_USER_DEFINED_DYNAMIC_OR_STATIC "STATIC")
ENDIF(DYNAMIC_OPENSCENEGRAPH)


# OSG Core
ADD_SUBDIRECTORY(src)

IF   (BUILD_OSG_APPLICATIONS)
    ADD_SUBDIRECTORY(applications)
ENDIF(BUILD_OSG_APPLICATIONS)

IF   (BUILD_OSG_EXAMPLES)
    ADD_SUBDIRECTORY(examples)
ENDIF(BUILD_OSG_EXAMPLES)


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
        IF(EXISTS /Developer/SDKs/MacOSX10.5.sdk)
            # 64-bit compiles are not supported with Carbon. We should enable 
            # 64-bit compilation by default once osgviewer has been 
            # rewritten with Cocoa.
            #SET(CMAKE_OSX_ARCHITECTURES "ppc;i386;ppc64;x86_64" CACHE STRING "Build architectures for OSX" FORCE)
            SET(CMAKE_OSX_ARCHITECTURES "ppc;i386" CACHE STRING "Build architectures for OSX" FORCE)
            SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mmacosx-version-min=10.5 -ftree-vectorize -fvisibility-inlines-hidden" CACHE STRING "Flags used by the compiler during all build types." FORCE)
        ELSE(EXISTS /Developer/SDKs/MacOSX10.5.sdk)
            IF(EXISTS /Developer/SDKs/MacOSX10.4u.sdk)
                SET(CMAKE_OSX_ARCHITECTURES "ppc;i386" CACHE STRING "Build architectures for OSX" FORCE)
                SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mmacosx-version-min=10.4 -ftree-vectorize -fvisibility-inlines-hidden" CACHE STRING "Flags used by the compiler during all build types." FORCE)
            ELSE(EXISTS /Developer/SDKs/MacOSX10.4u.sdk)
                # No Universal Binary support
                # Should break down further to set the -mmacosx-version-min,
                # but the SDK detection is too unreliable here.
            ENDIF(EXISTS /Developer/SDKs/MacOSX10.4u.sdk)
        ENDIF(EXISTS /Developer/SDKs/MacOSX10.5.sdk)
    ENDIF(NOT OSG_CONFIG_HAS_BEEN_RUN_BEFORE)

    OPTION(OSG_BUILD_APPLICATION_BUNDLES "Enable the building of applications and examples as OSX Bundles" OFF)

ENDIF(APPLE)


#
# Provide target for generating wrappers
#
SET(GENWRAPPER genwrapper)

ADD_CUSTOM_TARGET(wrappers
    COMMAND ${GENWRAPPER} -c ${OpenSceneGraph_SOURCE_DIR}/src/osgWrappers/genwrapper.conf -t ${OpenSceneGraph_SOURCE_DIR}/src/osgWrappers/Doxyfile.template -d ${OpenSceneGraph_SOURCE_DIR} | doxygen -
    COMMAND ${GENWRAPPER} -c ${OpenSceneGraph_SOURCE_DIR}/src/osgWrappers/genwrapper.conf -l ${OpenSceneGraph_SOURCE_DIR}
)

# For Doxygen
INCLUDE(${CMAKE_ROOT}/Modules/Documentation.cmake OPTIONAL)
OPTION(BUILD_DOCUMENTATION "Build OpenSceneGraph reference documentation using doxygen (use: make DoxygenDoc)" OFF)
MARK_AS_ADVANCED(CLEAR BUILD_DOCUMENTATION)
# To build the documention, you will have to enable it
# and then do the equivalent of "make DoxygenDoc".
IF(BUILD_DOCUMENTATION)

    OPTION(BUILD_REF_DOCS_SEARCHENGINE "Enable doxygen's search engine (requires that documentation to be installed on a php enabled web server)" OFF)
    IF(BUILD_REF_DOCS_SEARCHENGINE)
        SET(SEARCHENGINE YES)
    ELSE(BUILD_REF_DOCS_SEARCHENGINE)
        SET(SEARCHENGINE NO)
    ENDIF(BUILD_REF_DOCS_SEARCHENGINE)

    OPTION(BUILD_REF_DOCS_TAGFILE "Generate a tag file named osg.tag on the documentation web server" OFF)
    IF(BUILD_REF_DOCS_TAGFILE)
        SET(GENERATE_TAGFILE "${OpenSceneGraph_BINARY_DIR}/doc/OpenSceneGraphReferenceDocs/osg.tag")
    ELSE(BUILD_REF_DOCS_TAGFILE)
        SET(GENERATE_TAGFILE "")
    ENDIF(BUILD_REF_DOCS_TAGFILE)

    IF(DOT)
        SET(HAVE_DOT YES)
    ELSE(DOT)
        SET(HAVE_DOT NO)
    ENDIF(DOT)
    
    # If html help generation was requested. DOCUMENTATION_HTML_HELP is defined by Documentation.cmake
    SET(GENERATE_HTMLHELP "NO")
    IF(DOCUMENTATION_HTML_HELP)
        # on windows Documentation.cmake finds the html help workshop fi it exists. On u*ix we might have it with wine but no way to point it out
        IF(NOT WIN32)
            SET(HTML_HELP_COMPILER "" CACHE FILEPATH "Enter location of the HTML help compiler to let doxygen compile html")
            MARK_AS_ADVANCED(HTML_HELP_COMPILER)
        ENDIF(NOT WIN32)
        # this var sets a proper value in .doxygen files when coniguring them below
        SET(GENERATE_HTMLHELP "YES")
    endif(DOCUMENTATION_HTML_HELP)
  
    # This processes our doxyfile.cmake and substitutes paths to generate
    # a final Doxyfile
    CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/doc/Doxyfiles/doxyfile.cmake
        ${PROJECT_BINARY_DIR}/doc/openscenegraph.doxyfile
    )
    # copy the osg logo to documentations target folder
    CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/PlatformSpecifics/Windows/icons/src/osg32-32.png
        ${PROJECT_BINARY_DIR}/doc/OpenSceneGraphReferenceDocs/osg32-32.png COPYONLY
    )
    #INSTALL(FILES ${PROJECT_BINARY_DIR}/doc/${PROJECT_NAME}ReferenceDocs-${OPENSCENEGRAPH_VERSION}.chm DESTINATION doc OPTIONAL COMPONENT openscenegraph-doc)
    INSTALL(DIRECTORY ${PROJECT_BINARY_DIR}/doc/OpenSceneGraphReferenceDocs DESTINATION doc COMPONENT openscenegraph-doc)

    # now set up openthreads documentation generation
    IF(BUILD_REF_DOCS_TAGFILE)
        SET(GENERATE_TAGFILE "${OpenSceneGraph_BINARY_DIR}/doc/OpenThreadsReferenceDocs/ot.tag")
    ENDIF(BUILD_REF_DOCS_TAGFILE)

    # This processes our openthreads.doxyfile.cmake and generate a final doxyfile
    CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/doc/Doxyfiles/openthreads.doxyfile.cmake
        ${PROJECT_BINARY_DIR}/doc/openthreads.doxyfile
    )
    # copy the osg logo to documentations target folder
    CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/PlatformSpecifics/Windows/icons/src/osg32-32.png
        ${PROJECT_BINARY_DIR}/doc/OpenThreadsReferenceDocs/osg32-32.png COPYONLY
    )
    #INSTALL(FILES ${PROJECT_BINARY_DIR}/doc/${PROJECT_NAME}ReferenceDocs-${OPENSCENEGRAPH_VERSION}.chm DESTINATION doc OPTIONAL COMPONENT openscenegraph-doc)
    INSTALL(DIRECTORY ${PROJECT_BINARY_DIR}/doc/OpenThreadsReferenceDocs DESTINATION doc COMPONENT openthreads-doc)

    # Process our other doxyfiles but don't create targets for these
    CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/doc/Doxyfiles/all_Doxyfile
        ${PROJECT_BINARY_DIR}/doc/all_Doxyfile)
    CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/doc/Doxyfiles/auto_Doxyfile
        ${PROJECT_BINARY_DIR}/doc/auto_Doxyfile)
    CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/doc/Doxyfiles/core_Doxyfile
        ${PROJECT_BINARY_DIR}/doc/core_Doxyfile)
    
    # This creates a new target to build documentation.
    # It runs ${DOXYGEN} which is the full path and executable to
    # Doxygen on your system, set by the FindDoxygen.cmake module
    # (called by FindDocumentation.cmake).
    # It runs the final generated Doxyfile against it.
    # The DOT_PATH is substituted into the Doxyfile.
    ADD_CUSTOM_TARGET(doc_openscenegraph ${DOXYGEN}
        ${PROJECT_BINARY_DIR}/doc/openscenegraph.doxyfile
    )
    ADD_CUSTOM_TARGET(doc_openthreads ${DOXYGEN}
        ${PROJECT_BINARY_DIR}/doc/openthreads.doxyfile
    )
ENDIF(BUILD_DOCUMENTATION)

OPTION(BUILD_DASHBOARD_REPORTS "Set to ON to activate reporting of OpenSceneGraph builds here http://www.cdash.org/CDashPublic/index.php?project=OpenSceneGraph" OFF)
IF(BUILD_DASHBOARD_REPORTS)
# The following are required to uses Dart and the Cdash dashboard
# viewable here : http://www.cdash.org/CDashPublic/index.php?project=OpenSceneGraph
    INCLUDE(Dart)
ENDIF(BUILD_DASHBOARD_REPORTS)

# present the packaging option only if we have the cpack command defined (effectively >= 2.6.0)
IF(CMAKE_CPACK_COMMAND)
    OPTION(BUILD_OSG_PACKAGES "Set to ON to generate CPack configuration files and packaging targets" OFF)
    IF(BUILD_OSG_PACKAGES)
      INCLUDE(OsgCPack)
    ENDIF(BUILD_OSG_PACKAGES)
ENDIF(CMAKE_CPACK_COMMAND)

# Generate pkg-config configuration file

CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/packaging/pkgconfig/openscenegraph.pc.in
        ${PROJECT_BINARY_DIR}/packaging/pkgconfig/openscenegraph.pc
    @ONLY
    )

INSTALL(FILES ${PROJECT_BINARY_DIR}/packaging/pkgconfig/openscenegraph.pc DESTINATION lib${LIB_POSTFIX}/pkgconfig COMPONENT libopenscenegraph-dev)

# Run this as late as possible so users can easier spot the message
IF (${CMAKE_INSTALL_PREFIX} STREQUAL "/usr/local")
SET(REQUIRES_LIBPATH_MESSAGE ON)
ENDIF(${CMAKE_INSTALL_PREFIX} STREQUAL "/usr/local") 

IF(REQUIRES_LIBPATH_MESSAGE)

    IF (NOT OSG_LIBPATH_MESSAGE_HAS_BEEN_RUN_BEFORE)
 
        SET(OSG_LIBPATH_MESSAGE_HAS_BEEN_RUN_BEFORE 1 CACHE INTERNAL "Flag to track whether the libpath message has been reported before")

        MESSAGE("\nThe build system is configured to instal libraries to ${CMAKE_INSTALL_PREFIX}/lib${LIB_POSTFIX}\n"
            "Your applications may not be able to find your installed libraries unless you:\n"
            "    set your LD_LIBRARY_PATH (user specific) or\n"
            "    update your ld.so configuration (system wide)")
        IF(IS_DIRECTORY /etc/ld.so.conf.d)
            MESSAGE("You have an ld.so.conf.d directory on your system, so if you wish to ensure that\n"
                "applications find the installed osg libraries, system wide, you could install a\n"
                "openscenegraph specific ld.so configuration with:\n"
                "    sudo make install_ld_conf\n")
            CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/packaging/ld.so.conf.d/openscenegraph.conf.in
                ${PROJECT_BINARY_DIR}/packaging/ld.so.conf.d/openscenegraph.conf
            )
            ADD_CUSTOM_TARGET(install_ld_conf ${CMAKE_COMMAND} -E copy_if_different 
                ${PROJECT_BINARY_DIR}/packaging/ld.so.conf.d/openscenegraph.conf
                /etc/ld.so.conf.d/openscenegraph.conf
                COMMAND ldconfig
                COMMENT "Copying openscenegraph.conf to /etc/ld.so.conf.d and running ldconfig"
            )
        ELSE(IS_DIRECTORY /etc/ld.so.conf.d)
            IF(EXISTS /etc/ld.so.conf)
                MESSAGE("You have an ld.so.conf file in /etc, if you wish to ensure \n"
                "that applications find the installed osg libraries, system wide, you\n"
                "could add ${CMAKE_INSTALL_PREFIX}/lib${LIB_POSTFIX} to it.")
            ENDIF(EXISTS /etc/ld.so.conf)
        ENDIF(IS_DIRECTORY /etc/ld.so.conf.d)

        # emit a message during installation.
        INSTALL(CODE "MESSAGE(\"Libraries were installed to ${CMAKE_INSTALL_PREFIX}lib${LIB_POSTFIX}.\\nYou may need to update your ld.so configuration. \")")

    ENDIF(NOT OSG_LIBPATH_MESSAGE_HAS_BEEN_RUN_BEFORE)

ELSE(REQUIRES_LIBPATH_MESSAGE)

    SET(OSG_LIBPATH_MESSAGE_HAS_BEEN_RUN_BEFORE 0 CACHE INTERNAL "Flag to track whether the libpath message has been reported before")

ENDIF(REQUIRES_LIBPATH_MESSAGE)


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
