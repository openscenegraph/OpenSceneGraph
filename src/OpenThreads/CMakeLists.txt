# This is mainly for Windows declspec, but other platforms know
# what to do with it.
ADD_DEFINITIONS(-DOPENTHREADS_EXPORTS)

SET(OPENTHREADS_MAJOR_VERSION 2)
SET(OPENTHREADS_MINOR_VERSION 4)
SET(OPENTHREADS_PATCH_VERSION 0)
SET(OPENTHREADS_SOVERSION 11)

SET(OPENTHREADS_VERSION ${OPENTHREADS_MAJOR_VERSION}.${OPENTHREADS_MINOR_VERSION}.${OPENTHREADS_PATCH_VERSION})

INCLUDE(CheckAtomicOps)

# User Options
OPTION(DYNAMIC_OPENTHREADS "Set to ON to build OpenThreads for dynamic linking.  Use OFF for static." ON)
IF (DYNAMIC_OPENTHREADS)
    SET(OPENTHREADS_USER_DEFINED_DYNAMIC_OR_STATIC "SHARED")
ELSE (DYNAMIC_OPENTHREADS)
    SET(OPENTHREADS_USER_DEFINED_DYNAMIC_OR_STATIC "STATIC")
    SET(OT_LIBRARY_STATIC 1)
ENDIF (DYNAMIC_OPENTHREADS)

################################################################################
# Set Config file

SET(OPENTHREADS_CONFIG_HEADER "${PROJECT_BINARY_DIR}/include/OpenThreads/Config")
CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/common/Config.in"
               "${OPENTHREADS_CONFIG_HEADER}")

# INSTALL_FILES(/include/OpenThreads/ FILES "${OPENTHREADS_CONFIG_HEADER}")

################################################################################

SET(HEADER_PATH ${OpenThreads_SOURCE_DIR}/include/OpenThreads)
SET(OpenThreads_PUBLIC_HEADERS
    ${HEADER_PATH}/Atomic
    ${HEADER_PATH}/Barrier
    ${HEADER_PATH}/Block
    ${HEADER_PATH}/Condition
    ${HEADER_PATH}/Exports
    ${HEADER_PATH}/Mutex  
    ${HEADER_PATH}/ReadWriteMutex  
    ${HEADER_PATH}/ReentrantMutex  
    ${HEADER_PATH}/ScopedLock  
    ${HEADER_PATH}/Thread
    ${HEADER_PATH}/Version
    ${OPENTHREADS_CONFIG_HEADER}
)

# Use our modified version of FindThreads.cmake which has Sproc hacks.
FIND_PACKAGE(Threads)

# Do we have sproc?
IF(CMAKE_SYSTEM MATCHES IRIX)
    IF(CMAKE_USE_SPROC_INIT)
        # In this case, only Sproc exists, so no option.
        SET(OPENTHREADS_USE_SPROC_INSTEAD_OF_PTHREADS 1)
    ELSE(CMAKE_USE_SPROC_INIT)
        IF(CMAKE_HAVE_SPROC_H)
            OPTION(OPENTHREADS_USE_SPROC_INSTEAD_OF_PTHREADS "Set to ON to build OpenThreads against sproc instead of pthreads" OFF)
        ENDIF(CMAKE_HAVE_SPROC_H)
    ENDIF(CMAKE_USE_SPROC_INIT)
ENDIF(CMAKE_SYSTEM MATCHES IRIX)

# Maybe we should be using the FindThreads.cmake module?
IF(WIN32)
    # So I think Cygwin wants to use pthreads
    IF(CYGWIN)
        SUBDIRS(pthreads)
    ELSE(CYGWIN)
        # Everybody else including Msys should probably go here
        SUBDIRS(win32) 
        # examples)
    ENDIF(CYGWIN)
ELSE(WIN32)
    IF(UNIX)
        IF(OPENTHREADS_USE_SPROC_INSTEAD_OF_PTHREADS)
            SUBDIRS(sproc)
        ELSE(OPENTHREADS_USE_SPROC_INSTEAD_OF_PTHREADS)
            SUBDIRS(pthreads)
        ENDIF(OPENTHREADS_USE_SPROC_INSTEAD_OF_PTHREADS)
    ELSE(UNIX)
        MESSAGE("Sorry, OpenThreads may not support your platform")
    ENDIF(UNIX)
ENDIF(WIN32)

# Make sure everyone can find Config
INCLUDE_DIRECTORIES(${PROJECT_BINARY_DIR}/include)

# Generate pkg-config configuration file

CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/packaging/pkgconfig/openthreads.pc.in
        ${PROJECT_BINARY_DIR}/packaging/pkgconfig/openthreads.pc
    @ONLY
    )

INSTALL(FILES ${PROJECT_BINARY_DIR}/packaging/pkgconfig/openthreads.pc DESTINATION lib${LIB_POSTFIX}/pkgconfig  COMPONENT libopenthreads-dev)
