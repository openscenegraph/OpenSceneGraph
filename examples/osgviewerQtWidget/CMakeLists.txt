#user interface compilation
SET(SOURCES_UI testMainWin.ui testOutboardWin.ui)
QT4_WRAP_UI( SOURCES_UI_H ${SOURCES_UI})

#ressources
#SET(QtApp_RCCS ressources/images/icons.qrc)
#QT4_ADD_RESOURCES(QtApp_RCC_SRCS ${QtApp_RCCS})
 
# for the macro Q_OBJECT..
SET(SOURCES_H 
    CompositeViewerQOSG.h
    testMainWin.h
    testOutboardWin.h
)

QT4_WRAP_CPP( SOURCES_H_MOC ${SOURCES_H} )

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_BINARY_DIR} "./")

SET(TARGET_SRC 
    ${SOURCES_H_MOC} 
    ${SOURCES_UI_H}
    QOSGWidget.h
    testOutboardWin.cpp
    CompositeViewerQOSG.cpp
    QOSGWidget.cpp
    testMainWin.cpp
    main.cpp
)

IF   (QT4_FOUND)
    SET(TARGET_EXTERNAL_LIBRARIES ${QT_QTCORE_LIBRARY_RELEASE} ${QT_QTGUI_LIBRARY_RELEASE} ${QT_QTOPENGL_LIBRARY_RELEASE} )
    ADD_DEFINITIONS(-DUSE_QT4)
ELSE(QT4_FOUND)
    SET(TARGET_EXTERNAL_LIBRARIES ${QT_LIBRARIES} )
ENDIF(QT4_FOUND)

INCLUDE_DIRECTORIES(${QT_INCLUDE_DIR} )

#### end var setup  ###
SETUP_EXAMPLE(osgviewerQtWidget)

