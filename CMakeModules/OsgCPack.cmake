#
# collect a descriptive system specification string <system>-<arch>-<compiler>
#

# resolve architecture. The reason i "change" i686 to i386 is that debian packages
# require i386 so this is for the future
IF("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "i686")
    SET(SYSTEM_ARCH "i386")
ELSE("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "i686")
    SET(SYSTEM_ARCH ${CMAKE_SYSTEM_PROCESSOR})
ENDIF("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "i686")

# set a default system name - use CMake setting (Linux|Windows|...)
SET(SYSTEM_NAME ${CMAKE_SYSTEM_NAME})
#message(STATUS "CMAKE_SYSTEM_NAME ${CMAKE_SYSTEM_NAME}")
#message(STATUS "CMAKE_SYSTEM_PROCESSOR ${CMAKE_SYSTEM_PROCESSOR}")

SET(OSG_CPACK_COMPILER "" CACHE STRING "This ia short string (vc90, vc80sp1, g++-4.3, ...) describing your compiler. The string is used for creating package filenames")

# Windows specific settings. 
# For windows the compiler needs to be specified in the package filename
IF(WIN32)
    IF(MSVC)
        #message(STATUS "MSVC_VERSION ${MSVC_VERSION}")
        #Visual C++, 32-bit, version 6.0         1200
        #Visual C++, 32-bit, version .net 2002   1300
        #Visual C++, 32-bit, version .net 2003   1310
        #Visual C++, 32-bit, version 2005        1400 (vc80)
        #Visual C++, 32-bit, version 2005 SP1    14?? (vc80_sp1)
        #Visual C++, 32-bit, version 2008        1500 (vc90)

        IF(MSVC_VERSION EQUAL 1500)
            SET(OSG_CPACK_COMPILER "vc90")
        ENDIF(MSVC_VERSION EQUAL 1500)

        IF(MSVC_VERSION EQUAL 1400) # This doesn't work with my 2005 vc80sp1 compiler
            SET(OSG_CPACK_COMPILER "vc80")
        ELSE(MSVC_VERSION EQUAL 1400)
            IF(CMAKE_COMPILER_2005)
                SET(OSG_CPACK_COMPILER "vc80")
            ENDIF(CMAKE_COMPILER_2005)
        ENDIF(MSVC_VERSION EQUAL 1400)

        IF(MSVC_VERSION EQUAL 1310)
            SET(OSG_CPACK_COMPILER "vc70")
        ENDIF(MSVC_VERSION EQUAL 1310)

        # check arch bitcount and include this in the system name
        IF(CMAKE_CL_64)
            SET(SYSTEM_NAME "win64")
        ELSE(CMAKE_CL_64)
            SET(SYSTEM_NAME "win32")
        ENDIF(CMAKE_CL_64)
    ENDIF(MSVC)
ENDIF(WIN32)

IF(OSG_CPACK_COMPILER)
  SET(OSG_CPACK_SYSTEM_SPEC_STRING ${SYSTEM_NAME}-${SYSTEM_ARCH}-${OSG_CPACK_COMPILER})
ELSE(OSG_CPACK_COMPILER)
  SET(OSG_CPACK_SYSTEM_SPEC_STRING ${SYSTEM_NAME}-${SYSTEM_ARCH})
ENDIF(OSG_CPACK_COMPILER)

#message(STATUS "OSG_CPACK_SYSTEM_SPEC_STRING ${OSG_CPACK_SYSTEM_SPEC_STRING}")

# expose this to the user/packager
SET(CPACK_PACKAGE_CONTACT "" CACHE STRING "Supply contact information (email) here")
## variables that apply to all packages
SET(CPACK_PACKAGE_NAME ${CMAKE_PROJECT_NAME})
SET(CPACK_PACKAGE_FILE_NAME "openscenegraph-all-${OPENSCENEGRAPH_VERSION}-${OSG_CPACK_SYSTEM_SPEC_STRING}")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "The OpenSceneGraph is an open source high performance 3d graphics toolkit")
SET(CPACK_PACKAGE_VENDOR "The OpenSceneGraph authors")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${OpenSceneGraph_SOURCE_DIR}/README.txt")
SET(CPACK_RESOURCE_FILE_LICENSE "${OpenSceneGraph_SOURCE_DIR}/LICENSE.txt")
SET(CPACK_PACKAGE_VERSION_MAJOR ${OPENSCENEGRAPH_MAJOR_VERSION})
SET(CPACK_PACKAGE_VERSION_MINOR ${OPENSCENEGRAPH_MINOR_VERSION})
SET(CPACK_PACKAGE_VERSION_PATCH ${OPENSCENEGRAPH_PATCH_VERSION})
SET(CPACK_PACKAGE_INSTALL_DIRECTORY "OpenSceneGraph-${OPENSCENEGRAPH_MAJOR_VERSION}.${OPENSCENEGRAPH_MINOR_VERSION}")
SET(CPACK_TOPLEVEL_TAG ${CPACK_PACKAGE_NAME}-${OPENSCENEGRAPH_VERSION})
SET(CPACK_SOURCE_TOPLEVEL_TAG ${CPACK_PACKAGE_NAME}-${OPENSCENEGRAPH_VERSION}-src)

SET(CPACK_SOURCE_PACKAGE_FILE_NAME "openscenegraph-${OPENSCENEGRAPH_VERSION}-src")
# Add the build directory to the ignore patterns and expose var to the users 
# N.B. This is especially important if your are building out-of-source but under the source tree (i.e <OpenSceneGraphSrcDir>/build).
#     If you don't name your builddir here it will get pulled into the src package. Size of my build tree is gigabytes so you dont want this
SET(CPACK_SOURCE_IGNORE_FILES "/\\\\\\\\.svn/;\\\\\\\\.swp$;\\\\\\\\.#;/#;build" CACHE STRING "Add ignore patterns that will left out of the src package")

# these goes for all platforms. Setting these stops the CPack.cmake script from generating options about other package compression formats (.z .tz, etc.)
SET(CPACK_GENERATOR "TGZ")
SET(CPACK_SOURCE_GENERATOR "TGZ")

IF(WIN32 AND NOT UNIX)
    
    OPTION(BUILD_NSIS_PACKAGE "Turn this ON if you want to generate a visual installer using NSIS (nsis.sourceforge.net)" OFF)
    IF(BUILD_NSIS_PACKAGE)
        # There is a bug in NSIS that does not handle full unix paths properly. Make
        # sure there is at least one set of four (4) backlasshes.
        SET(CPACK_PACKAGE_ICON "${OpenSceneGraph_SOURCE_DIR}/PlatformSpecifics/Windows/icons\\\\osg.ico")
        SET(CPACK_NSIS_INSTALLED_ICON_NAME "")
        SET(CPACK_NSIS_DISPLAY_NAME "OpenSceneGraph ${OPENSCENEGRAPH_VERSION}")
        SET(CPACK_NSIS_HELP_LINK "http:\\\\\\\\www.openscenegraph.org/projects/osg/wiki/Support")
        SET(CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\www.openscenegraph.org/projects/osg/wiki/About")
        SET(CPACK_NSIS_CONTACT "")
        SET(CPACK_NSIS_MODIFY_PATH ON)

        SET(CPACK_GENERATOR "NSIS")
    ENDIF(BUILD_NSIS_PACKAGE)
ELSE(WIN32 AND NOT UNIX)
    SET(CPACK_STRIP_FILES ON)
    SET(CPACK_SOURCE_STRIP_FILES ON)
ENDIF(WIN32 AND NOT UNIX)

#STRING(TOLOWER "${CMAKE_SYSTEM_NAME}" CPACK_SYSTEM_NAME)

# Including CPack will generate CPackConfig.cmake and CPackSourceConfig.cmake and make targets regardless of how i call it
# The first idea was to not use it at all and that might be where we're going. For now it also defines some useful macros, especially 
# for the visual installers, so I decided to include it to have the possibility to create visual installers for ms and mac and then try to
# make the best use I could of the targets that including CPack implies
INCLUDE(CPack)

# including CPack also has the benefit of creating this nice variable which is a collection of all defined COMPONENTS
# Create configs and targets for each component
FOREACH(package ${CPACK_COMPONENTS_ALL})
    # the doc packages don't need a system-arch specification
    IF(${package} MATCHES -doc)
        SET(CPACK_PACKAGE_FILE_NAME ${package}-${OPENSCENEGRAPH_VERSION})
    ELSE(${package} MATCHES -doc)
        SET(CPACK_PACKAGE_FILE_NAME ${package}-${OPENSCENEGRAPH_VERSION}-${OSG_CPACK_SYSTEM_SPEC_STRING})
    ENDIF(${package} MATCHES -doc)

    SET(OSG_CPACK_COMPONENT ${package})
    CONFIGURE_FILE("${OpenSceneGraph_SOURCE_DIR}/CMakeModules/OsgCPackConfig.cmake.in" "${OpenSceneGraph_BINARY_DIR}/CPackConfig-${OSG_CPACK_COMPONENT}.cmake" IMMEDIATE)

    ADD_CUSTOM_TARGET("package_${package}" 
        COMMAND ${CMAKE_CPACK_COMMAND} --config ${OpenSceneGraph_BINARY_DIR}/CPackConfig-${OSG_CPACK_COMPONENT}.cmake
        COMMENT Run CPack packaging for ${package}...
    )
ENDFOREACH(package ${CPACK_COMPONENTS_ALL})
