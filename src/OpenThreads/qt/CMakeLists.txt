# This file should only be included when using Qt threads

SET(LIB_NAME OpenThreads)
SET(LIB_PUBLIC_HEADERS ${OpenThreads_PUBLIC_HEADERS})

SOURCE_GROUP("Header Files" FILES ${LIB_PUBLIC_HEADERS})
SET_SOURCE_FILES_PROPERTIES(${LIB_PUBLIC_HEADERS} PROPERTIES HEADER_FILE_ONLY ON)

SET(QTCLASS_HEADERS
    QtThreadPrivateData.h
    QtMutexPrivateData.h
    QtConditionPrivateData.h
    QtBarrierPrivateData.h
)

# No need to moc headers right now
#QT4_WRAP_CPP( QTCLASS_MOC ${QTCLASS_HEADERS} OPTIONS "-f" )

ADD_LIBRARY(${LIB_NAME}
    ${OPENTHREADS_USER_DEFINED_DYNAMIC_OR_STATIC}
    ${LIB_PUBLIC_HEADERS}
    ${QTCLASS_HEADERS}
    QtThread.cpp
    QtMutex.cpp
    QtCondition.cpp
    QtBarrier.cpp
    ../common/Version.cpp
    ../common/Atomic.cpp
    ${QTCLASS_MOC}
    ${OPENTHREADS_VERSIONINFO_RC}
)

IF(CMAKE_COMPILER_IS_GNUCXX)
    # Remove -pedantic flag as it barfs on Qt headers
   STRING(REGEX REPLACE "-pedantic" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
ENDIF()

IF(OPENTHREADS_SONAMES)
  SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES VERSION ${OPENTHREADS_VERSION} SOVERSION ${OPENTHREADS_SOVERSION})
ENDIF()

if ( QT4_FOUND )
    TARGET_LINK_LIBRARIES(${LIB_NAME} ${QT_QTCORE_LIBRARY})
    INCLUDE_DIRECTORIES(${QT_INCLUDE_DIR} ${QT_QTCORE_INCLUDE_DIR})
else( QT4_FOUND )
IF ( Qt5Widgets_FOUND )
    qt5_use_modules( ${LIB_NAME} Core )
ENDIF ( Qt5Widgets_FOUND )
    
endif( QT4_FOUND )

# Since we're building different platforms binaries in 
# their respective directories, we need to set the 
# link directory so it can find this location.
LINK_DIRECTORIES(
    ${CMAKE_CURRENT_BINARY_DIR}
)

IF(MSVC AND OSG_MSVC_VERSIONED_DLL)
    HANDLE_MSVC_DLL(ot ${OPENTHREADS_SOVERSION})
ENDIF()

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
