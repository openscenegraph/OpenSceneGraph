# This file should only be included when using Sproc

SET(LIB_NAME OpenThreads)
SET(LIB_PUBLIC_HEADERS ${OpenThreads_PUBLIC_HEADERS})

ADD_LIBRARY(${LIB_NAME}
    ${OPENTHREADS_USER_DEFINED_DYNAMIC_OR_STATIC}
    ${LIB_PUBLIC_HEADERS}
    SharedArena.c++
    SharedArena.h
    SprocBarrier.c++
    SprocBarrierPrivateData.h
    SprocCondition.c++
    SprocConditionPrivateData.h
    SprocMutex.c++
    SprocMutexPrivateData.h
    SprocThread.c++
    SprocThreadPrivateActions.h
    SprocThreadPrivateData.h
    ../common/Version.cpp
    ../common/Atomic.cpp
)

IF(OPENTHREADS_SONAMES)
  SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES VERSION ${OPENTHREADS_VERSION} SOVERSION ${OPENTHREADS_SOVERSION})
ENDIF()

# Do we need to link against anything for Sproc?
#TARGET_LINK_LIBRARIES(${LIB_NAME}
#)

# Since we're building different platforms binaries in 
# their respective directories, we need to set the 
# link directory so it can find this location.
LINK_DIRECTORIES(
    ${CMAKE_CURRENT_BINARY_DIR}
)

INSTALL(
    TARGETS OpenThreads
    ARCHIVE DESTINATION lib${LIB_POSTFIX} COMPONENT libopenthreads-dev
    LIBRARY DESTINATION lib${LIB_POSTFIX} COMPONENT libopenthreads
    RUNTIME DESTINATION bin COMPONENT libopenthreads
)
INSTALL(
    FILES ${OpenThreads_PUBLIC_HEADERS}
    DESTINATION include/OpenThreads
    COMPONENT libopenthreads-dev
)

#commented out# INCLUDE(ModuleInstall OPTIONAL)
