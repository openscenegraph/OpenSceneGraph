
# FIXME: For OS X, need flag for Framework or dylib
IF   (DYNAMIC_OPENSCENEGRAPH)
    ADD_DEFINITIONS(-DOSGVIEWER_LIBRARY)
ELSE (DYNAMIC_OPENSCENEGRAPH)
    ADD_DEFINITIONS(-DOSG_LIBRARY_STATIC)
ENDIF(DYNAMIC_OPENSCENEGRAPH)

SET(LIB_NAME osgViewer)
SET(HEADER_PATH ${OpenSceneGraph_SOURCE_DIR}/include/${LIB_NAME})
SET(LIB_PUBLIC_HEADERS
    ${HEADER_PATH}/CompositeViewer
    ${HEADER_PATH}/Export
    ${HEADER_PATH}/GraphicsWindow
    ${HEADER_PATH}/Renderer
    ${HEADER_PATH}/Scene
    ${HEADER_PATH}/Version
    ${HEADER_PATH}/View
    ${HEADER_PATH}/Viewer
    ${HEADER_PATH}/ViewerBase
    ${HEADER_PATH}/ViewerEventHandlers
)

SET(LIB_COMMON_FILES
    CompositeViewer.cpp
    HelpHandler.cpp
    Renderer.cpp
    Scene.cpp
    ScreenCaptureHandler.cpp
    StatsHandler.cpp
    Version.cpp
    View.cpp
    Viewer.cpp
    ViewerBase.cpp
    ViewerEventHandlers.cpp
)

SET(LIB_EXTRA_LIBS)

IF(WIN32)

    SET(LIB_PUBLIC_HEADERS ${LIB_PUBLIC_HEADERS}
        ${HEADER_PATH}/api/Win32/GraphicsWindowWin32
        ${HEADER_PATH}/api/Win32/PixelBufferWin32
    )
        
    SET(LIB_COMMON_FILES ${LIB_COMMON_FILES} 
        GraphicsWindowWin32.cpp
        PixelBufferWin32.cpp
    )

ELSE(WIN32)

    IF(APPLE)
        SET(OSG_WINDOWING_SYSTEM "Carbon" CACHE STRING "Windowing system type for graphics window creation, options Carbon or X11.")
    ELSE(APPLE)
        SET(OSG_WINDOWING_SYSTEM "X11" CACHE STRING "Windowing system type for graphics window creation. options only X11")
    ENDIF(APPLE)
    

    IF(${OSG_WINDOWING_SYSTEM} STREQUAL "Carbon")

        # FIXME: OS X needs selection mechanism for Cocoa, Carbon, X11
        SET(LIB_PUBLIC_HEADERS ${LIB_PUBLIC_HEADERS} 
            ${HEADER_PATH}/api/Carbon/GraphicsWindowCarbon
            ${HEADER_PATH}/api/Carbon/PixelBufferCarbon
        )
        SET(LIB_COMMON_FILES ${LIB_COMMON_FILES} 
            GraphicsWindowCarbon.cpp
            PixelBufferCarbon.cpp
        )

    ELSE(${OSG_WINDOWING_SYSTEM} STREQUAL "Carbon")

        # X11 for everybody else
        OPTION(OSGVIEWER_USE_XRANDR "Set to ON to enable Xrandr support for GraphicsWindowX11." OFF)

        SET(LIB_PUBLIC_HEADERS ${LIB_PUBLIC_HEADERS}
            ${HEADER_PATH}/api/X11/GraphicsWindowX11
            ${HEADER_PATH}/api/X11/PixelBufferX11
            ${XRANDR_INCLUDE_DIRS}
        )
        SET(LIB_COMMON_FILES ${LIB_COMMON_FILES} 
            GraphicsWindowX11.cpp 
            PixelBufferX11.cpp
        )
        
        IF(OSGVIEWER_USE_XRANDR)
            INCLUDE(FindPkgConfig)
            PKG_CHECK_MODULES(XRANDR REQUIRED xrandr)
            ADD_DEFINITIONS(-DOSGVIEWER_USE_XRANDR)
            SET(LIB_PUBLIC_HEADERS ${LIB_PUBLIC_HEADERS} ${XRANDR_INCLUDE_DIRS})
            LINK_LIBRARIES(Xrandr)
        ENDIF(OSGVIEWER_USE_XRANDR)

        # X11 on Apple requires X11 library plus OpenGL linking hack on Leopard
        IF(APPLE)
            # Find GL/glx.h 
            IF(EXISTS ${CMAKE_OSX_SYSROOT}/usr/X11/include/GL/glx.h)
                SET(OPENGL_INCLUDE_DIR /usr/X11/include)
                SET(OPENGL_LIBRARIES /usr/X11/lib/libGL.dylib)
            ELSEIF(EXISTS ${CMAKE_OSX_SYSROOT}/usr/X11R6/include/GL/glx.h)
                SET(OPENGL_INCLUDE_DIR /usr/X11R6/include)
                SET(OPENGL_LIBRARIES /usr/X11R6/lib/libGL.dylib)
            ENDIF(EXISTS ${CMAKE_OSX_SYSROOT}/usr/X11/include/GL/glx.h)
            INCLUDE_DIRECTORIES(BEFORE SYSTEM ${OPENGL_INCLUDE_DIR})

            SET(LIB_EXTRA_LIBS ${X11_X11_LIB} ${OPENGL_LIBRARIES} ${LIB_EXTRA_LIBS})
            SET(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,-dylib_file,/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib:${CMAKE_OSX_SYSROOT}/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib")
        ENDIF(APPLE)
            

    ENDIF(${OSG_WINDOWING_SYSTEM} STREQUAL "Carbon")

ENDIF(WIN32)


ADD_LIBRARY(${LIB_NAME}
    ${OPENSCENEGRAPH_USER_DEFINED_DYNAMIC_OR_STATIC}
    ${LIB_PUBLIC_HEADERS}
    ${LIB_PRIVATE_HEADERS}
    ${LIB_COMMON_FILES}
)


LINK_INTERNAL(${LIB_NAME}     
    osgGA
    osgText
    osgDB
    osgUtil
    osg
    OpenThreads
    ${LIB_EXTRA_LIBS}
)

LINK_CORELIB_DEFAULT(${LIB_NAME})

IF(MINGW OR CYGWIN)
    LINK_EXTERNAL(${LIB_NAME} gdi32 )
ENDIF(MINGW OR CYGWIN)

INCLUDE(ModuleInstall OPTIONAL)

FOREACH( INCLUDEFILE ${LIB_PUBLIC_HEADERS} )
   FILE(RELATIVE_PATH REL_INCLUDEFILE ${HEADER_PATH} ${INCLUDEFILE})
   GET_FILENAME_COMPONENT(REL_INCLUDE_PATH ${REL_INCLUDEFILE} PATH)
   INSTALL(
   FILES        ${INCLUDEFILE}
   DESTINATION ${INSTALL_INCDIR}/${LIB_NAME}/${REL_INCLUDE_PATH}
   )
ENDFOREACH( INCLUDEFILE)
