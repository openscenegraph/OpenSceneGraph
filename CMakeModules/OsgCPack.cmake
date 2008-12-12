
## variables that apply to all packages
SET(CPACK_PACKAGE_NAME ${CMAKE_PROJECT_NAME})
SET(CPACK_PACKAGE_FILE_NAME "openscenegraph-${OPENSCENEGRAPH_VERSION}")
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

# platform specifics. Per default generate zips on win32 and tgz's on unix
IF(APPLE)
    # don't really know how to do it on the MAC yet
ELSE(APPLE)
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
        ELSE(BUILD_NSIS_PACKAGE)
            SET(CPACK_GENERATOR "ZIP")
        ENDIF(BUILD_NSIS_PACKAGE)
    ELSE(WIN32 AND NOT UNIX)
        SET(CPACK_STRIP_FILES ON)
        SET(CPACK_SOURCE_STRIP_FILES ON)

        SET(CPACK_GENERATOR "TGZ")
    ENDIF(WIN32 AND NOT UNIX)
ENDIF(APPLE)

STRING(TOLOWER "${CMAKE_SYSTEM_NAME}" CPACK_SYSTEM_NAME)

# include CPack will generate CPackConfig.cmake and CPackSourceConfig.cmake
# Including CPack will generate CPackConfig.cmake and CPackSourceConfig.cmake and make targets regardless of how i call it
# The first idea was to not use it at all and that might be where we're going. For now it also defines some useful macros, especially 
# for the visual installers, so I decided to include it to have the possibility to create visual installers for ms and mac and then try to
# make the best use I could of the targets that including CPack implies
include(CPack)

# includiong CPack will generate a PACKAGE project on MSVC and package/package_src target on unixes. For MSVC I also create a PACACKGE_SRC
IF(MSVC_IDE)
    add_custom_target("PACKAGE-SOURCE" 
        COMMAND ${CMAKE_CPACK_COMMAND} --config ${OpenSceneGraph_BINARY_DIR}/CPackSourceConfig.cmake
    )
ENDIF(MSVC_IDE)

# including CPack also has the benefit of creating this nice var which is a collection of all defined COMPONENTS
# Create configs and targets for each component
FOREACH(package ${CPACK_COMPONENTS_ALL})
    set(CPACK_PACKAGE_FILE_NAME ${package}-${OPENSCENEGRAPH_VERSION}-${CPACK_SYSTEM_NAME})
    set(OSG_CPACK_COMPONENT ${package})
    configure_file("${OpenSceneGraph_SOURCE_DIR}/CMakeModules/OsgCPackConfig.cmake.in" "${OpenSceneGraph_BINARY_DIR}/CPackConfig-${OSG_CPACK_COMPONENT}.cmake" IMMEDIATE)

    add_custom_target("package_${package}" 
        COMMAND ${CMAKE_CPACK_COMMAND} --config ${OpenSceneGraph_BINARY_DIR}/CPackConfig-${OSG_CPACK_COMPONENT}.cmake
        COMMENT Run CPack packaging for ${package}...
    )
ENDFOREACH(package ${CPACK_COMPONENTS_ALL})
