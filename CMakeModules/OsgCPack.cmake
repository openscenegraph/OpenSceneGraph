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

# Platform specific settings. 
# Includes setting the compiler and specifying debug/release build
# for windows the SYSTEM_NAME is set win32/64 instead of windows
IF(MSVC)
    #Visual C++, 32-bit, version 6.0         1200
    #Visual C++, 32-bit, version .net 2002   1300
    #Visual C++, 32-bit, version .net 2003   1310
    #Visual C++, 32-bit, version 2005        1400 (vc80)
    #Visual C++, 32-bit, version 2005 SP1    1400 (vc80_sp1)
    #Visual C++, 32-bit, version 2008        1500 (vc90)

    IF(MSVC_VERSION EQUAL 1500)
        SET(OSG_CPACK_COMPILER "vc90")
    ELSE(MSVC_VERSION EQUAL 1500)
        # This if doesn't detect my 2005 vc80sp1 compiler. Have to rely on COMPILER_2005
        IF(MSVC_VERSION EQUAL 1400) 
            SET(OSG_CPACK_COMPILER "vc80")
        ELSE(MSVC_VERSION EQUAL 1400)
            IF(CMAKE_COMPILER_2005)
                SET(OSG_CPACK_COMPILER "vc80")
            ENDIF(CMAKE_COMPILER_2005)
        ENDIF(MSVC_VERSION EQUAL 1400)
    ENDIF(MSVC_VERSION EQUAL 1500)

    IF(MSVC_VERSION EQUAL 1310)
        SET(OSG_CPACK_COMPILER "vc70")
    ENDIF(MSVC_VERSION EQUAL 1310)

    # check arch bitcount
    IF(CMAKE_CL_64)
        SET(SYSTEM_NAME "win64")
    ELSE(CMAKE_CL_64)
        SET(SYSTEM_NAME "win32")
    ENDIF(CMAKE_CL_64)
ELSE(MSVC)
    # on un*x an empty CMAKE_BUILD_TYPE means release
    IF(CMAKE_BUILD_TYPE)
        SET(SYSTEM_BUILD_TYPE ${CMAKE_BUILD_TYPE})
    ELSE(CMAKE_BUILD_TYPE)
        SET(SYSTEM_BUILD_TYPE "Release")
    ENDIF(CMAKE_BUILD_TYPE)
ENDIF(MSVC)

# expose the compiler setting to the user
SET(OSG_CPACK_COMPILER "${OSG_CPACK_COMPILER}" CACHE STRING "This ia short string (vc90, vc80sp1, gcc-4.3, ...) describing your compiler. The string is used for creating package filenames")

IF(OSG_CPACK_COMPILER)
  SET(OSG_CPACK_SYSTEM_SPEC_STRING ${SYSTEM_NAME}-${SYSTEM_ARCH}-${OSG_CPACK_COMPILER})
ELSE(OSG_CPACK_COMPILER)
  SET(OSG_CPACK_SYSTEM_SPEC_STRING ${SYSTEM_NAME}-${SYSTEM_ARCH})
ENDIF(OSG_CPACK_COMPILER)


## variables that apply to all packages
SET(CPACK_PACKAGE_NAME ${CMAKE_PROJECT_NAME})
#SET(CPACK_PACKAGE_FILE_NAME "openscenegraph-all-${OPENSCENEGRAPH_VERSION}-${OSG_CPACK_SYSTEM_SPEC_STRING}")
SET(CPACK_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-${OPENSCENEGRAPH_MAJOR_VERSION}.${OPENSCENEGRAPH_MINOR_VERSION}")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "The OpenSceneGraph is an open source high performance 3d graphics toolkit")
SET(CPACK_PACKAGE_VENDOR "The OpenSceneGraph developers and contributors lead by Robert Osfield")
SET(CPACK_PACKAGE_VERSION_MAJOR ${OPENSCENEGRAPH_MAJOR_VERSION})
SET(CPACK_PACKAGE_VERSION_MINOR ${OPENSCENEGRAPH_MINOR_VERSION})
SET(CPACK_PACKAGE_VERSION_PATCH ${OPENSCENEGRAPH_PATCH_VERSION})
SET(CPACK_PACKAGE_INSTALL_DIRECTORY "OpenSceneGraph-${OPENSCENEGRAPH_MAJOR_VERSION}.${OPENSCENEGRAPH_MINOR_VERSION}")
SET(CPACK_TOPLEVEL_TAG ${CPACK_PACKAGE_NAME}-${OPENSCENEGRAPH_VERSION})


# these goes for all platforms. Setting these stops the CPack.cmake script from generating options about other package compression formats (.z .tz, etc.)
SET(CPACK_GENERATOR "TGZ")
SET(CPACK_SOURCE_GENERATOR "TGZ")


# for msvc we use it's internally defined variable to get the configuration (debug,release, ...) 
IF(MSVC)
    SET(OSG_CPACK_CONFIGURATION "$(OutDir)")
    SET(PACKAGE_TARGET_PREFIX "Package ")
ELSE(MSVC)
    SET(OSG_CPACK_CONFIGURATION "${SYSTEM_BUILD_TYPE}")
    SET(PACKAGE_TARGET_PREFIX "package_")
ENDIF(MSVC)

# Get all defined components
IF(CMAKE_MAJOR_VERSION EQUAL 2 AND CMAKE_MINOR_VERSION GREATER 4 AND CMAKE_PATCH_VERSION GREATER 0)
  GET_CMAKE_PROPERTY(CPACK_COMPONENTS_ALL COMPONENTS)
ELSE(CMAKE_MAJOR_VERSION EQUAL 2 AND CMAKE_MINOR_VERSION GREATER 4 AND CMAKE_PATCH_VERSION GREATER 0)
  # cmake 2.6.0 didn't supply the COMPONENTS property.
  # I set it manually to be the packages that can always be packaged
  MESSAGE("When building packages please consider using cmake version 2.6.1 or above")
  SET(CPACK_COMPONENTS_ALL libopenscenegraph openscenegraph libopenthreads libopenscenegraph-dev libopenthreads-dev)
ENDIF(CMAKE_MAJOR_VERSION EQUAL 2 AND CMAKE_MINOR_VERSION GREATER 4 AND CMAKE_PATCH_VERSION GREATER 0)

# Create a target that will be used to generate all packages defined below
SET(PACKAGE_ALL_TARGETNAME "${PACKAGE_TARGET_PREFIX}ALL")
ADD_CUSTOM_TARGET(${PACKAGE_ALL_TARGETNAME})

MACRO(GENERATE_PACKAGING_TARGET package_name)
    SET(CPACK_PACKAGE_NAME ${package_name})

    # the doc packages don't need a system-arch specification
    IF(${package} MATCHES -doc)
        SET(OSG_PACKAGE_FILE_NAME ${package_name}-${OPENSCENEGRAPH_VERSION})
    ELSE(${package} MATCHES -doc)
        SET(OSG_PACKAGE_FILE_NAME ${package_name}-${OPENSCENEGRAPH_VERSION}-${OSG_CPACK_SYSTEM_SPEC_STRING}-${OSG_CPACK_CONFIGURATION})
    ENDIF(${package} MATCHES -doc)

    CONFIGURE_FILE("${OpenSceneGraph_SOURCE_DIR}/CMakeModules/OsgCPackConfig.cmake.in" "${OpenSceneGraph_BINARY_DIR}/CPackConfig-${package_name}.cmake" IMMEDIATE)

    SET(PACKAGE_TARGETNAME "${PACKAGE_TARGET_PREFIX}${package_name}")
    # Create a target that creates the current package
    ADD_CUSTOM_TARGET(${PACKAGE_TARGETNAME})
    ADD_CUSTOM_COMMAND(TARGET ${PACKAGE_TARGETNAME}
        COMMAND ${CMAKE_CPACK_COMMAND} -C ${OSG_CPACK_CONFIGURATION} --config ${OpenSceneGraph_BINARY_DIR}/CPackConfig-${package_name}.cmake
        COMMENT "Run CPack packaging for ${package_name}..."
    )
    ADD_CUSTOM_COMMAND(TARGET ${PACKAGE_ALL_TARGETNAME}
        COMMAND ${CMAKE_CPACK_COMMAND} -C ${OSG_CPACK_CONFIGURATION} --config ${OpenSceneGraph_BINARY_DIR}/CPackConfig-${package_name}.cmake
    )
    
    # This is naive and will probably need fixing eventually
    IF(MSVC)
        SET(MOVE_COMMAND "move")
    ELSE(MSVC)
        SET(MOVE_COMMAND "mv")
    ENDIF(MSVC)
    
    # Rename the package to get the proper filename <package>-<platform>-<arch>[compiler]-<buildtype>.tar.gz
    ADD_CUSTOM_COMMAND(TARGET ${PACKAGE_TARGETNAME}
#        COMMAND "${MOVE_COMMAND}" "${CPACK_PACKAGE_FILE_NAME}.tar.gz" "${OSG_PACKAGE_FILE_NAME}-$(OutDir).tar.gz"
        COMMAND "${MOVE_COMMAND}" "${CPACK_PACKAGE_FILE_NAME}.tar.gz" "${OSG_PACKAGE_FILE_NAME}.tar.gz"
    )
    # Add the exact same custom command to the all package generating target. 
    # I can't use add_dependencies to do this because it would allow parallell building of packages so am going brute here
    ADD_CUSTOM_COMMAND(TARGET ${PACKAGE_ALL_TARGETNAME}
        COMMAND "${MOVE_COMMAND}" "${CPACK_PACKAGE_FILE_NAME}.tar.gz" "${OSG_PACKAGE_FILE_NAME}.tar.gz"
    )
ENDMACRO(GENERATE_PACKAGING_TARGET)

# Create configs and targets for a package including all components
SET(OSG_CPACK_COMPONENT ALL)
GENERATE_PACKAGING_TARGET(openscenegraph-all)

# Create configs and targets for each component
FOREACH(package ${CPACK_COMPONENTS_ALL})
    SET(OSG_CPACK_COMPONENT ${package})
    GENERATE_PACKAGING_TARGET(${package})
ENDFOREACH(package ${CPACK_COMPONENTS_ALL})
