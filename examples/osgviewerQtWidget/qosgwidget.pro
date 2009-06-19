# Adjust these for your build environment.
win32 {
    OSGHOME=c:/Program Files/OpenSceneGraph
} else {
    OSGHOME=/usr/osg/OpenSceneGraph-2.8.1
    #OUT_OF_SOURCE_POSTFIX=
    # OUT_OF_SOURCE_POSTFIX=.build_debug
    OUT_OF_SOURCE_POSTFIX=.build_release
}

D=
# Uncomment to enable debug library linkinig
# CONFIG += debug 
# D=d

#-----------------------------------------------------------------------------
# This is untested...
# Added for Qt-4.5.1 which now requires linking with Frameworks
# /usr/local/Trolltech/Qt-4.5.1/lib/QtGui.framework/QtGui
#
# QMAKE_FLAGS += -F/usr/local/Trolltech/Qt-4.5.1/lib/
# LIBS += -framework QtGui -framework QtCore

#-----------------------------------------------------------------------------
TEMPLATE = app
QMAKE_CXXFLAGS_THREAD += -pthread
CONFIG += thread

#-----------------------------------------------------------------------------
win32 {
    cpu_type = win32_x86
} else {

    cpu_type = $$system(uname -m)
    OBJECTS_DIR = $${cpu_type}
    TARGET  = $${OBJECTS_DIR}/qosgwidget
}

#-----------------------------------------------------------------------------
win32 {
    CONFIG += console
    DEPENDPATH  += .
    INCLUDEPATH += . $$quote("\"$${OSGHOME}/include\"")

} else {
    # Need the extra path to find OpenThreads/Config with out-of-source builds.
    INCLUDEPATH += $${OSGHOME}$${OUT_OF_SOURCE_POSTFIX}/include $${OSGHOME}/include 
}

macx {
    CONFIG += x86
    # Uncomment to turn off qDebug() messages:
    # When uncommented there's a problem with a Frameworks header file??!!
    # DEFINES += QT_NO_DEBUG_OUTPUT
} else {
    # Comment out to turn on qDebug() messages:
    DEFINES += QT_NO_DEBUG_OUTPUT
}


FORMS = testMainWin.ui testOutboardWin.ui

HEADERS = testMainWin.h testOutboardWin.h QOSGWidget.h CompositeViewerQOSG.h

SOURCES += main.cpp testMainWin.cpp testOutboardWin.cpp QOSGWidget.cpp \
        CompositeViewerQOSG.cpp

OSG_LIBS=  -losgText$${D} -losgGA$${D} -losgFX$${D} \
           -losgDB$${D} -losgUtil$${D} -losg$${D} \
           -lOpenThreads$${D} -losgViewer$${D} \

#-----------------------------------------------------------------------------
win32 {
    OSG_LIB_DIR=$${OSGHOME}/lib
    QMAKE_LFLAGS += /LIBPATH:$$quote("\"$${OSG_LIB_DIR}\"")
    LIBS +=    $${OSG_LIBS}
} else {
    LIBS += -L$${OSGHOME}$${OUT_OF_SOURCE_POSTFIX}/lib $${OSG_LIBS} 
}

