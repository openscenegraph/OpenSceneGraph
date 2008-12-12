# This file should only be included when WIN32

SET(LIB_NAME OpenThreads)
SET(LIB_PUBLIC_HEADERS ${OpenThreads_PUBLIC_HEADERS})

SOURCE_GROUP("Header Files" FILES ${LIB_PUBLIC_HEADERS})
SET_SOURCE_FILES_PROPERTIES(${LIB_PUBLIC_HEADERS} PROPERTIES HEADER_FILE_ONLY ON)

ADD_LIBRARY(${LIB_NAME}
    ${OPENTHREADS_USER_DEFINED_DYNAMIC_OR_STATIC}
    ${LIB_PUBLIC_HEADERS}
    HandleHolder.h
    Win32BarrierPrivateData.h
    WIN32Condition.cpp
    Win32Condition.h
    Win32ConditionPrivateData.h
    Win32Mutex.cpp
    Win32MutexPrivateData.h
    Win32Thread.cpp
    Win32ThreadBarrier.cpp
    Win32ThreadPrivateData.h
    ../common/Version.cpp
    ../common/Atomic.cpp
)


IF(OPENTHREADS_SONAMES)
  SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES VERSION ${OPENTHREADS_VERSION} SOVERSION ${OPENTHREADS_SOVERSION})
ENDIF(OPENTHREADS_SONAMES)

# Since we're building different platforms binaries in 
# their respective directories, we need to set the 
# link directory so it can find this location.
LINK_DIRECTORIES(
    ${CMAKE_CURRENT_BINARY_DIR}
)

IF(MSVC AND OSG_MSVC_VERSIONED_DLL)
    HANDLE_MSVC_DLL(ot ${OPENTHREADS_SOVERSION})
ENDIF(MSVC AND OSG_MSVC_VERSIONED_DLL)

INSTALL(
    TARGETS OpenThreads
    ARCHIVE DESTINATION lib COMPONENT libopenthreads-dev
    LIBRARY DESTINATION lib COMPONENT libopenthreads
    RUNTIME DESTINATION bin COMPONENT libopenthreads
)
INSTALL(
    FILES ${OpenThreads_PUBLIC_HEADERS}
    DESTINATION include/OpenThreads
    COMPONENT libopenthreads-dev
)

#commented out# INCLUDE(ModuleInstall OPTIONAL)
