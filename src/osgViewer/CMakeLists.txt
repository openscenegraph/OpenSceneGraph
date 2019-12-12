# FIXME: For OS X, need flag for Framework or dylib
IF(DYNAMIC_OPENSCENEGRAPH)
    ADD_DEFINITIONS(-DOSGVIEWER_LIBRARY)
ELSE()
    ADD_DEFINITIONS(-DOSG_LIBRARY_STATIC)
ENDIF()

SET(LIB_NAME osgViewer)


################################################################################
## Quieten warnings that a due to optional code paths

IF(CMAKE_COMPILER_IS_GNUCXX)
    # Remove -Wextra flag as it barfs on ffmoeg headers
   STRING(REGEX REPLACE "-Wextra" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
ENDIF()

SET(HEADER_PATH ${OpenSceneGraph_SOURCE_DIR}/include/${LIB_NAME})


FILE(GLOB CONFIG_HEADER_FILES ${HEADER_PATH}/config/*)
FILE(GLOB CONFIG_SOURCE_FILES config/*.cpp)

SET(TARGET_H
    ${HEADER_PATH}/CompositeViewer
    ${HEADER_PATH}/Export
    ${HEADER_PATH}/GraphicsWindow
    ${HEADER_PATH}/Keystone
    ${HEADER_PATH}/Renderer
    ${HEADER_PATH}/Scene
    ${HEADER_PATH}/Version
    ${HEADER_PATH}/View
    ${HEADER_PATH}/Viewer
    ${HEADER_PATH}/ViewerBase
    ${HEADER_PATH}/ViewerEventHandlers
)

SET(LIB_COMMON_FILES
    ${CONFIG_SOURCE_FILES}
    CompositeViewer.cpp
    GraphicsWindow.cpp
    HelpHandler.cpp
    Keystone.cpp
    Renderer.cpp
    Scene.cpp
    ScreenCaptureHandler.cpp
    StatsHandler.cpp
    Version.cpp
    View.cpp
    Viewer.cpp
    ViewerBase.cpp
    ViewerEventHandlers.cpp
    ${OPENSCENEGRAPH_VERSIONINFO_RC}
)

# Collect all the configuration files
SET(LIB_EXTRA_LIBS)

IF(${OSG_WINDOWING_SYSTEM} STREQUAL "Win32")
    #
    # Enable workaround for OpenGL driver issues when used in multithreaded/multiscreen with NVidia drivers on Windows XP
    # For example: osgviewer dumptruck.osg was showing total garbage (screen looked like shattered, splashed hedgehog)
    # There were also serious issues with render to texture cameras.
    # Workaround repeats makeCurrentContext call as it was found that this causes the problems to disappear.
    #
    OPTION(OSG_MULTIMONITOR_MULTITHREAD_WIN32_NVIDIA_WORKAROUND "Set to ON if you have NVidia board and drivers earlier than 177.92 ver" OFF)
    MARK_AS_ADVANCED(OSG_MULTIMONITOR_MULTITHREAD_WIN32_NVIDIA_WORKAROUND)
    IF(OSG_MULTIMONITOR_MULTITHREAD_WIN32_NVIDIA_WORKAROUND)
        ADD_DEFINITIONS(-DOSG_MULTIMONITOR_MULTITHREAD_WIN32_NVIDIA_WORKAROUND)
    ENDIF()

    SET(TARGET_H_NO_MODULE_INSTALL
        ${HEADER_PATH}/api/Win32/GraphicsHandleWin32
        ${HEADER_PATH}/api/Win32/GraphicsWindowWin32
        ${HEADER_PATH}/api/Win32/PixelBufferWin32
    )

    SET(LIB_COMMON_FILES ${LIB_COMMON_FILES}
        GraphicsWindowWin32.cpp
        PixelBufferWin32.cpp
    )
ELSEIF(${OSG_WINDOWING_SYSTEM} STREQUAL "Cocoa")
    ADD_DEFINITIONS(-DUSE_DARWIN_COCOA_IMPLEMENTATION)

    IF(OSG_COMPILE_FRAMEWORKS)
       SET(LIB_COMMON_FILES ${LIB_COMMON_FILES}
             ${HEADER_PATH}/api/Cocoa/GraphicsHandleCocoa
             ${HEADER_PATH}/api/Cocoa/GraphicsWindowCocoa
             ${HEADER_PATH}/api/Cocoa/PixelBufferCocoa
         )
        SET_PROPERTY(SOURCE ${HEADER_PATH}/api/Cocoa/GraphicsHandleCocoa PROPERTY MACOSX_PACKAGE_LOCATION Headers/api/Cocoa)
        SET_PROPERTY(SOURCE ${HEADER_PATH}/api/Cocoa/GraphicsWindowCocoa PROPERTY MACOSX_PACKAGE_LOCATION Headers/api/Cocoa)
        SET_PROPERTY(SOURCE ${HEADER_PATH}/api/Cocoa/PixelBufferCocoa PROPERTY MACOSX_PACKAGE_LOCATION Headers/api/Cocoa)
    ELSE()
        SET(TARGET_H_NO_MODULE_INSTALL
            ${HEADER_PATH}/api/Cocoa/GraphicsHandleCocoa
            ${HEADER_PATH}/api/Cocoa/GraphicsWindowCocoa
            ${HEADER_PATH}/api/Cocoa/PixelBufferCocoa
        )
    ENDIF()

    SET(LIB_COMMON_FILES ${LIB_COMMON_FILES}
        GraphicsWindowCocoa.mm
        DarwinUtils.h
        DarwinUtils.mm
        PixelBufferCocoa.mm
    )
    SET(LIB_EXTRA_LIBS ${COCOA_LIBRARY} ${LIB_EXTRA_LIBS})

ELSEIF(${OSG_WINDOWING_SYSTEM} STREQUAL "Carbon")
    ADD_DEFINITIONS(-DUSE_DARWIN_CARBON_IMPLEMENTATION)

    IF(OSG_COMPILE_FRAMEWORKS)
       SET(LIB_COMMON_FILES ${LIB_COMMON_FILES}
             ${HEADER_PATH}/api/Carbon/GraphicsHandleCarbon
             ${HEADER_PATH}/api/Carbon/GraphicsWindowCarbon
             ${HEADER_PATH}/api/Carbon/PixelBufferCarbon
         )
        SET_PROPERTY(SOURCE ${HEADER_PATH}/api/Carbon/GraphicsHandleCarbon PROPERTY MACOSX_PACKAGE_LOCATION Headers/api/Carbon)
        SET_PROPERTY(SOURCE ${HEADER_PATH}/api/Carbon/GraphicsWindowCarbon PROPERTY MACOSX_PACKAGE_LOCATION Headers/api/Carbon)
        SET_PROPERTY(SOURCE ${HEADER_PATH}/api/Carbon/PixelBufferCarbon PROPERTY MACOSX_PACKAGE_LOCATION Headers/api/Carbon)
    ELSE()
        SET(TARGET_H_NO_MODULE_INSTALL
            ${HEADER_PATH}/api/Carbon/GraphicsHandleCarbon
            ${HEADER_PATH}/api/Carbon/GraphicsWindowCarbon
            ${HEADER_PATH}/api/Carbon/PixelBufferCarbon
        )
    ENDIF()

    SET(LIB_COMMON_FILES ${LIB_COMMON_FILES}
        GraphicsWindowCarbon.cpp
        DarwinUtils.h
        DarwinUtils.mm
        PixelBufferCarbon.cpp
    )

    SET(LIB_EXTRA_LIBS ${COCOA_LIBRARY} ${AGL_LIBRARY} ${LIB_EXTRA_LIBS})

ELSEIF(${OSG_WINDOWING_SYSTEM} STREQUAL "IOS")

    ADD_DEFINITIONS(-DUSE_IOS_IMPLEMENTATION)

    IF(OSG_COMPILE_FRAMEWORKS)
       SET(LIB_COMMON_FILES ${LIB_COMMON_FILES}
             ${HEADER_PATH}/api/IOS/GraphicsWindowIOS
         )

        SET_PROPERTY(SOURCE ${HEADER_PATH}/api/IOS/GraphicsWindowIOS PROPERTY MACOSX_PACKAGE_LOCATION Headers/api/IOS)
    ELSE()
        SET(TARGET_H_NO_MODULE_INSTALL
            ${HEADER_PATH}/api/IOS/GraphicsWindowIOS
            #${HEADER_PATH}/api/IOS/PixelBufferIOS
        )
    ENDIF()

    SET(LIB_COMMON_FILES ${LIB_COMMON_FILES}
        GraphicsWindowIOS.mm
        IOSUtils.h
        IOSUtils.mm
    )

    SET(LIB_EXTRA_LIBS ${COCOA_LIBRARY} ${LIB_EXTRA_LIBS})

ELSEIF(${OSG_WINDOWING_SYSTEM} STREQUAL "X11")
    # X11 for everybody else
    INCLUDE(FindPkgConfig OPTIONAL)
    IF(PKG_CONFIG_FOUND)

        PKG_CHECK_MODULES(XRANDR xrandr)
        IF(XRANDR_FOUND)
            OPTION(OSGVIEWER_USE_XRANDR "Set to ON to enable Xrandr support for GraphicsWindowX11." ON)
        ELSE()
            SET(OSGVIEWER_USE_XRANDR OFF)
        ENDIF()

        PKG_CHECK_MODULES(XINERAMA xinerama)
        IF(XINERAMA_FOUND)
            SET(OSGVIEWER_USE_XINERAMA ON)
        ELSE()
            SET(OSGVIEWER_USE_XINERAMA OFF)
        ENDIF()
    ELSE()
        SET(OSGVIEWER_USE_XRANDR OFF)
        SET(OSGVIEWER_USE_XINERAMA OFF)
    ENDIF()

    SET(TARGET_H_NO_MODULE_INSTALL
        ${HEADER_PATH}/api/X11/GraphicsHandleX11
        ${HEADER_PATH}/api/X11/GraphicsWindowX11
        ${HEADER_PATH}/api/X11/PixelBufferX11
    )

    SET(LIB_COMMON_FILES ${LIB_COMMON_FILES}
        GraphicsWindowX11.cpp
        PixelBufferX11.cpp
    )

    IF(OSGVIEWER_USE_XRANDR)
        ADD_DEFINITIONS(-DOSGVIEWER_USE_XRANDR)
        SET(LIB_PRIVATE_HEADERS ${LIB_PRIVATE_HEADERS} ${XRANDR_INCLUDE_DIRS} )

        IF(X11_Xrandr_LIB)
            SET(LIB_EXTRA_LIBS ${X11_Xrandr_LIB} ${LIB_EXTRA_LIBS})
        ELSE()
            SET(LIB_EXTRA_LIBS ${XRANDR_LIBRARIES} ${LIB_EXTRA_LIBS})
        ENDIF()
    ENDIF()

    IF(OSGVIEWER_USE_XINERAMA)
        ADD_DEFINITIONS(-DOSGVIEWER_USE_XINERAMA)
        SET(LIB_PRIVATE_HEADERS ${LIB_PRIVATE_HEADERS} ${XINERAMA_INCLUDE_DIRS} )
        SET(LIB_EXTRA_LIBS ${X11_Xinerama_LIB} ${LIB_EXTRA_LIBS})
    ENDIF()

    # X11 on Apple requires X11 library plus OpenGL linking hack on Leopard
    IF(APPLE)
        # hack for finding the iphone opengl es lib
        IF(OSG_BUILD_PLATFORM_IPHONE)
          SET(OPENGL_INCLUDE_DIR ${IPHONE_SDKROOT}/System/Library/Frameworks)
          SET(OPENGL_LIBRARIES ${IPHONE_SDKROOT}/System/Library/Frameworks/OpenGLES)
        ELSE()
          SET(LIB_EXTRA_LIBS ${X11_X11_LIB} ${LIB_EXTRA_LIBS})
        ENDIF()

    ELSE(APPLE)

      SET(LIB_EXTRA_LIBS ${X11_X11_LIB} ${LIB_EXTRA_LIBS})
    ENDIF(APPLE)
ELSE()
    MESSAGE(STATUS "Windowing system not supported")
ENDIF()


SET(TARGET_SRC
    ${LIB_PRIVATE_HEADERS}
    ${LIB_COMMON_FILES}
)


SET(TARGET_LIBRARIES
    osgGA
    osgText
    osgDB
    osgUtil
    osg
    OpenThreads
)

SET(TARGET_EXTERNAL_LIBRARIES ${LIB_EXTRA_LIBS})

IF(MINGW OR CYGWIN)
    SET(TARGET_EXTERNAL_LIBRARIES ${TARGET_EXTERNAL_LIBRARIES} gdi32 )
ENDIF()

SETUP_LIBRARY(${LIB_NAME})

# install these headers manually since setup_library and moduleinstall
# wouldn't keep the structure, e.g. api/X11/xyz.h
FOREACH(HEADERFILE ${TARGET_H_NO_MODULE_INSTALL} ${CONFIG_HEADER_FILES})
  # get relative path to graphics system dependent header
  FILE(RELATIVE_PATH REL_INCLUDEFILE ${HEADER_PATH} ${HEADERFILE})
  GET_FILENAME_COMPONENT(REL_INCLUDE_PATH ${REL_INCLUDEFILE} PATH)

  INSTALL(FILES ${HEADERFILE}
    DESTINATION ${INSTALL_INCDIR}/${LIB_NAME}/${REL_INCLUDE_PATH}
    COMPONENT libopenscenegraph-dev
  )
ENDFOREACH()

# for structure in ide
SOURCE_GROUP("${HEADERS_GROUP}\\api" FILES ${TARGET_H_NO_MODULE_INSTALL})
