# This script sets up packaging targets for each "COMPONENT" as specified in INSTALL commands
#
# for each component a CPackConfig-<component>.cmake is generated in the build tree
# and a target is added to call cpack for it (e.g. package_openscenegaph
# A target for generating a package with everything that gets INSTALLED is generated (package_openscenegraph-all)
# A target for making all of the abaove packages is generated (package_ALL)
#
# package filenames are created on the form <package>-<platform>-<arch>[-<compiler>]-<build_type>[-static].tar.gz
# ...where compiler optionally set using a cmake gui (OSG_CPACK_COMPILER). This script tries to guess compiler version for msvc generators
# ...build_type matches CMAKE_BUILD_TYPE for all generators but the msvc ones

# resolve architecture. The reason i "change" i686 to i386 is that debian packages
# require i386 so this is for the future
IF("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "i686")
    SET(SYSTEM_ARCH "i386")
ELSE()
    SET(SYSTEM_ARCH ${CMAKE_SYSTEM_PROCESSOR})
ENDIF()

# set a default system name - use CMake setting (Linux|Windows|...)
SET(SYSTEM_NAME ${CMAKE_SYSTEM_NAME})
#message(STATUS "CMAKE_SYSTEM_NAME ${CMAKE_SYSTEM_NAME}")
#message(STATUS "CMAKE_SYSTEM_PROCESSOR ${CMAKE_SYSTEM_PROCESSOR}")

# for msvc the SYSTEM_NAME is set win32/64 instead of "Windows"
IF(MSVC)
    IF(CMAKE_CL_64)
        SET(SYSTEM_NAME "win64")
    ELSE()
        SET(SYSTEM_NAME "win32")
    ENDIF()
ENDIF()
# Guess the compiler (is this desired for other platforms than windows?)
IF(NOT DEFINED OSG_CPACK_COMPILER)
    INCLUDE(OsgDetermineCompiler)
ENDIF()

# expose the compiler setting to the user
SET(OSG_CPACK_COMPILER "${OSG_COMPILER}" CACHE STRING "This ia short string (vc90, vc80sp1, gcc-4.3, ...) describing your compiler. The string is used for creating package filenames")

IF(OSG_CPACK_COMPILER)
  SET(OSG_CPACK_SYSTEM_SPEC_STRING ${SYSTEM_NAME}-${SYSTEM_ARCH}-${OSG_CPACK_COMPILER})
ELSE()
  SET(OSG_CPACK_SYSTEM_SPEC_STRING ${SYSTEM_NAME}-${SYSTEM_ARCH})
ENDIF()


## variables that apply to all packages
SET(CPACK_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-${OPENSCENEGRAPH_VERSION}")

# these goes for all platforms. Setting these stops the CPack.cmake script from generating options about other package compression formats (.z .tz, etc.)
IF(WIN32)
    SET(CPACK_GENERATOR "ZIP" CACHE STRING "CPack package generator type (i.e ZIP,NSIS,TGZ,DEB,RPM, -- see CPack for valid stypes")
ELSE()
    SET(CPACK_GENERATOR "TGZ" CACHE STRING "CPack package generator type (i.e ZIP,NSIS,TGZ,DEB,RPM, -- see CPack for valid stypes")
ENDIF()
SET(CPACK_SOURCE_GENERATOR "TGZ")


# for ms visual studio we use it's internally defined variable to get the configuration (debug,release, ...)
IF(MSVC_IDE)
    SET(OSG_CPACK_CONFIGURATION "$(OutDir)")
    SET(PACKAGE_TARGET_PREFIX "Package ")
ELSE()
    # on un*x an empty CMAKE_BUILD_TYPE means release
    IF(CMAKE_BUILD_TYPE)
        SET(OSG_CPACK_CONFIGURATION ${CMAKE_BUILD_TYPE})
    ELSE()
        SET(OSG_CPACK_CONFIGURATION "Release")
    ENDIF()
    SET(PACKAGE_TARGET_PREFIX "package_")
ENDIF()

# Get all defined components
GET_CMAKE_PROPERTY(CPACK_COMPONENTS_ALL COMPONENTS)
IF(NOT CPACK_COMPONENTS_ALL)
  # cmake 2.6.0 don't supply the COMPONENTS property.
  # I set it manually to be the packages that can always be packaged
  MESSAGE("When building packages please consider using cmake version 2.6.1 or above")
  SET(CPACK_COMPONENTS_ALL libopenscenegraph libopenthreads openscenegraph libopenscenegraph-dev libopenthreads-dev)
ENDIF()

# Create a target that will be used to generate all packages defined below
SET(PACKAGE_ALL_TARGETNAME "${PACKAGE_TARGET_PREFIX}ALL")
ADD_CUSTOM_TARGET(${PACKAGE_ALL_TARGETNAME})

 # cpack configuration for debian packages
IF(${CPACK_GENERATOR} STREQUAL "DEB")
    SET(OPENSCENEGRAPH_PACKAGE_MAINTAINER
        ""
        CACHE STRING
        "Name and email address of the package maintainer, e.g., 'Jon Doe <jon.doe@superawesomemail.com>'"
    )
    SET(CPACK_LIBOPENSCENEGRAPH_DEPENDENCIES
        "libopenthreads"
        CACHE STRING
        "Dependend packages for the openscenegraph library package (uses deb dependecy format), e.g., 'libc6, libcurl3-gnutls, libgif4, libjpeg8, libpng12-0'"
    )
    SET(CPACK_LIBOPENSCENEGRAPH-DEV_DEPENDENCIES
        "libopenscenegraph"
        CACHE STRING
        "Dependend packages for the openscenegraph development package (uses deb dependecy format), e.g., 'libc6, libcurl3-gnutls, libgif4, libjpeg8, libpng12-0'"
    )
    SET(CPACK_LIBOPENTHREADS_DEPENDENCIES
        ""
        CACHE STRING
        "Dependend packages for the openthreads library package (uses deb dependecy format), e.g., 'libc6, libcurl3-gnutls, libgif4, libjpeg8, libpng12-0'"
    )
    SET(CPACK_LIBOPENTHREADS-DEV_DEPENDENCIES
        "libopenthreads"
        CACHE STRING
        "Dependend packages for the openthreads development package (uses deb dependecy format), e.g., 'libc6, libcurl3-gnutls, libgif4, libjpeg8, libpng12-0'"
    )
    SET(CPACK_OPENSCENEGRAPH_DEPENDENCIES
        "libopenscenegraph"
        CACHE STRING
        "Dependend packages for the openscenegraph main package (uses deb dependecy format), e.g., 'libc6, libcurl3-gnutls, libgif4, libjpeg8, libpng12-0'"
    )
    SET(CPACK_OPENSCENEGRAPH-ALL_DEPENDENCIES
        ""
        CACHE STRING
        "Dependend packages for the openscenegraph package with all components (uses deb dependecy format), e.g., 'libc6, libcurl3-gnutls, libgif4, libjpeg8, libpng12-0'"
    )

    SET(CPACK_LIBOPENSCENEGRAPH_CONFLICTS
        ""
        CACHE STRING
        "Conflicting packages for the openscenegraph library package (uses deb dependecy format), e.g., 'libc6, libcurl3-gnutls, libgif4, libjpeg8, libpng12-0'"
    )
    SET(CPACK_LIBOPENSCENEGRAPH-DEV_CONFLICTS
        ""
        CACHE STRING
        "Conflicting packages for the openscenegraph development package (uses deb dependecy format), e.g., 'libc6, libcurl3-gnutls, libgif4, libjpeg8, libpng12-0'"
    )
    SET(CPACK_LIBOPENTHREADS_CONFLICTS
        ""
        CACHE STRING
        "Conflicting packages for the openthreads library package (uses deb dependecy format), e.g., 'libc6, libcurl3-gnutls, libgif4, libjpeg8, libpng12-0'"
    )
    SET(CPACK_LIBOPENTHREADS-DEV_CONFLICTS
        ""
        CACHE STRING
        "Conflicting packages for the openthreads development package (uses deb dependecy format), e.g., 'libc6, libcurl3-gnutls, libgif4, libjpeg8, libpng12-0'"
    )
    SET(CPACK_OPENSCENEGRAPH_CONFLICTS
        ""
        CACHE STRING
        "Conflicting packages for the openscenegraph main package (uses deb dependecy format), e.g., 'libc6, libcurl3-gnutls, libgif4, libjpeg8, libpng12-0'"
    )
    SET(CPACK_OPENSCENEGRAPH-ALL_CONFLICTS
        ""
        CACHE STRING
        "Conflicting packages for the openscenegraph package with all components (uses deb dependecy format), e.g., 'libc6, libcurl3-gnutls, libgif4, libjpeg8, libpng12-0'"
    )
ENDIF()

MACRO(GENERATE_PACKAGING_TARGET package_name)
    SET(CPACK_PACKAGE_NAME ${package_name})

    # set debian dependencies AND conflicts
    IF(${CPACK_GENERATOR} STREQUAL "DEB")
        STRING(TOUPPER CPACK_${package_name}_DEPENDENCIES DEPENDENCIES_VAR)
        STRING(TOUPPER CPACK_${package_name}_CONFLICTS CONFLICTS_VAR)
        SET(OSG_PACKAGE_DEPENDS "${${DEPENDENCIES_VAR}}")
        SET(OSG_PACKAGE_CONFLICTS "${${CONFLICTS_VAR}}")
    ENDIF()

    # the doc packages don't need a system-arch specification
    IF(${package} MATCHES -doc)
        SET(OSG_PACKAGE_FILE_NAME ${package_name}-${OPENSCENEGRAPH_VERSION})
    ELSE()
        SET(OSG_PACKAGE_FILE_NAME ${package_name}-${OPENSCENEGRAPH_VERSION}-${OSG_CPACK_SYSTEM_SPEC_STRING}-${OSG_CPACK_CONFIGURATION})
        IF(NOT DYNAMIC_OPENSCENEGRAPH)
            SET(OSG_PACKAGE_FILE_NAME ${OSG_PACKAGE_FILE_NAME}-static)
        ENDIF()
    ENDIF()

    CONFIGURE_FILE("${OpenSceneGraph_SOURCE_DIR}/CMakeModules/OsgCPackConfig.cmake.in" "${OpenSceneGraph_BINARY_DIR}/CPackConfig-${package_name}.cmake" IMMEDIATE)

    SET(PACKAGE_TARGETNAME "${PACKAGE_TARGET_PREFIX}${package_name}")

    # This is naive and will probably need fixing eventually
    IF(MSVC)
        SET(MOVE_COMMAND "move")
    ELSE()
        SET(MOVE_COMMAND "mv")
    ENDIF()

    # Set in and out archive filenames. Windows = zip, others = tar.gz
    IF(WIN32)
        SET(ARCHIVE_EXT "zip")
    ELSE()
        SET(ARCHIVE_EXT "tar.gz")
    ENDIF()

    # Create a target that creates the current package
    # and rename the package to give it proper filename
    ADD_CUSTOM_TARGET(${PACKAGE_TARGETNAME})
    SET_TARGET_PROPERTIES(${PACKAGE_TARGETNAME} PROPERTIES FOLDER "Packaging")

    ADD_CUSTOM_COMMAND(TARGET ${PACKAGE_TARGETNAME}
        COMMAND ${CMAKE_CPACK_COMMAND} -C ${OSG_CPACK_CONFIGURATION} --config ${OpenSceneGraph_BINARY_DIR}/CPackConfig-${package_name}.cmake
        COMMENT "Run CPack packaging for ${package_name}..."
    )
    # Add the exact same custom command to the all package generating target.
    # I can't use add_dependencies to do this because it would allow parallel building of packages so am going brute here
    ADD_CUSTOM_COMMAND(TARGET ${PACKAGE_ALL_TARGETNAME}
        COMMAND ${CMAKE_CPACK_COMMAND} -C ${OSG_CPACK_CONFIGURATION} --config ${OpenSceneGraph_BINARY_DIR}/CPackConfig-${package_name}.cmake
    )
    SET_TARGET_PROPERTIES(${PACKAGE_ALL_TARGETNAME} PROPERTIES FOLDER "Packaging")

ENDMACRO(GENERATE_PACKAGING_TARGET)

# Create configs and targets for a package including all components
SET(OSG_CPACK_COMPONENT ALL)
GENERATE_PACKAGING_TARGET(openscenegraph-all)

# Create configs and targets for each component
FOREACH(package ${CPACK_COMPONENTS_ALL})
    SET(OSG_CPACK_COMPONENT ${package})
    GENERATE_PACKAGING_TARGET(${package})
ENDFOREACH()
