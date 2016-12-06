# This is mainly for Windows declspec, but other platforms know
# what to do with it.
ADD_DEFINITIONS(-DOPENTHREADS_EXPORTS)

SET(OPENTHREADS_MAJOR_VERSION 3)
SET(OPENTHREADS_MINOR_VERSION 3)
SET(OPENTHREADS_PATCH_VERSION 1)
SET(OPENTHREADS_SOVERSION 21)

SET(OPENTHREADS_VERSION ${OPENTHREADS_MAJOR_VERSION}.${OPENTHREADS_MINOR_VERSION}.${OPENTHREADS_PATCH_VERSION})

IF(NOT ANDROID)
    INCLUDE(CheckAtomicOps)
ENDIF()

# User Options
OPTION(DYNAMIC_OPENTHREADS "Set to ON to build OpenThreads for dynamic linking.  Use OFF for static." ON)
IF(DYNAMIC_OPENTHREADS)
    SET(OPENTHREADS_USER_DEFINED_DYNAMIC_OR_STATIC "SHARED")
ELSE()
    SET(OPENTHREADS_USER_DEFINED_DYNAMIC_OR_STATIC "STATIC")
    SET(OT_LIBRARY_STATIC 1)
ENDIF()

################################################################################
# Set Config file

SET(OPENTHREADS_CONFIG_HEADER "${PROJECT_BINARY_DIR}/include/OpenThreads/Config")
CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/common/Config.in"
               "${OPENTHREADS_CONFIG_HEADER}")

SET(OPENTHREADS_VERSION_HEADER "${PROJECT_BINARY_DIR}/include/OpenThreads/Version")
CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/common/Version.in"
               "${OPENTHREADS_VERSION_HEADER}")


################################################################################
# Set Version Info resource file

IF(MSVC)
    SET(OPENTHREADS_VERSIONINFO_RC "${PROJECT_BINARY_DIR}/PlatformSpecifics/Windows/OpenThreadsVersionInfo.rc")
    CONFIGURE_FILE("${PROJECT_SOURCE_DIR}/PlatformSpecifics/Windows/OpenThreadsVersionInfo.rc.in"
                   "${OPENTHREADS_VERSIONINFO_RC}")
ENDIF()


################################################################################
## Quieten warnings that a due to optional code paths

IF(CMAKE_COMPILER_IS_GNUCXX)
    # Remove -Wextra flag as it barfs on ffmoeg headers
   STRING(REGEX REPLACE "-Wextra" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
ENDIF()

# INSTALL_FILES(/include/OpenThreads/ FILES "${OPENTHREADS_CONFIG_HEADER}")

################################################################################

SET(HEADER_PATH ${OpenThreads_SOURCE_DIR}/include/OpenThreads)
SET(OpenThreads_PUBLIC_HEADERS
    ${HEADER_PATH}/Atomic
    ${HEADER_PATH}/Affinity
    ${HEADER_PATH}/Barrier
    ${HEADER_PATH}/Block
    ${HEADER_PATH}/Condition
    ${HEADER_PATH}/Exports
    ${HEADER_PATH}/Mutex
    ${HEADER_PATH}/ReadWriteMutex
    ${HEADER_PATH}/ReentrantMutex
    ${HEADER_PATH}/ScopedLock
    ${HEADER_PATH}/Thread
    ${OPENTHREADS_VERSION_HEADER}
    ${OPENTHREADS_CONFIG_HEADER}
)

IF(NOT ANDROID)
# Use our modified version of FindThreads.cmake which has Sproc hacks.
    FIND_PACKAGE(Threads)
ENDIF()

# Add support for using Qt threads
IF(QT4_FOUND OR Qt5Widgets_FOUND)
    OPTION(BUILD_OPENTHREADS_WITH_QT "Build OpenThreads with Qt threading support." OFF)
ENDIF()

# Maybe we should be using the FindThreads.cmake module?
IF(ANDROID)
    SUBDIRS(pthreads)
ELSEIF(WIN32)
    # So I think Cygwin wants to use pthreads
    IF(CYGWIN)
        SUBDIRS(pthreads)
    ELSE()
        # Everybody else including Msys should probably go here
        SUBDIRS(win32)
        # examples)
    ENDIF()
ELSE()
    IF(UNIX)
        SUBDIRS(pthreads)
    ELSE()
        MESSAGE("Sorry, OpenThreads may not support your platform")
    ENDIF()
ENDIF()

# Make sure everyone can find Config
INCLUDE_DIRECTORIES(${PROJECT_BINARY_DIR}/include)

# Generate pkg-config configuration file

CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/packaging/pkgconfig/openthreads.pc.in
        ${PROJECT_BINARY_DIR}/packaging/pkgconfig/openthreads.pc
    @ONLY
    )

INSTALL(FILES ${PROJECT_BINARY_DIR}/packaging/pkgconfig/openthreads.pc DESTINATION lib${LIB_POSTFIX}/pkgconfig  COMPONENT libopenthreads-dev)
