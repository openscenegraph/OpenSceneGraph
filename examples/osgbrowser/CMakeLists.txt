SET(TARGET_SRC
    llembeddedbrowser.cpp
    llembeddedbrowserwindow.cpp
    llmozlib2.cpp
    nsProfileDirServiceProvider.cpp
    osgbrowser.cpp
)

SET(TARGET_H
    llembeddedbrowser.h
    llembeddedbrowserwindow.h
    llmozlib2.h
    nsProfileDirServiceProvider.h
    nsProfileLock.h
    nsProfileStringTypes.h
)

ADD_DEFINITIONS(-DMOZILLA_INTERNAL_API)

# MESSAGE("XUL_LIB_DIRS = " ${XUL_LIB_DIRS})
# MESSAGE("XUL_LIBRARIES = " ${XUL_LIBRARIES})
# MESSAGE("XUL_INCLUDE_DIRS = " ${XUL_INCLUDE_DIRS})

INCLUDE_DIRECTORIES( ${GLUT_INCLUDE_DIR}
    ${GTK_INCLUDE_DIRS} ${GTKGL_INCLUDE_DIRS}
    ${XUL_INCLUDE_DIRS} 
    /usr/include/xulrunner/locale 
    /usr/include/xulrunner/view 
    /usr/include/xulrunner/content 
    /usr/include/xulrunner/gfx 
    /usr/include/xulrunner/layout 
    /usr/include/xulrunner/webbrwsr 
    /usr/include/xulrunner/widget  
    /usr/include/xulrunner/dom 
    /usr/include/xulrunner/docshell 
    /usr/include/xulrunner/pref 
    /usr/include/xulrunner/nkcache 
    /usr/include/xulrunner/necko/ 
    /usr/include/xulrunner/profdirserviceprovider 
    /usr/include/xulrunner/nkcache 
    /usr/include/xulrunner/xulapp
    /usr/include/xulrunner/profile 
    /usr/include/xulrunner/appshell
    /usr/include/xulrunner/xpcom_obsolete
    /usr/include/xulrunner/intl
    /usr/include/xulrunner/windowwatcher
    /usr/include/xulrunner/xpconnect
)
LINK_DIRECTORIES( ${XUL_LIB_DIRS})
SET(TARGET_EXTERNAL_LIBRARIES ${XUL_LIBRARIES} ${GLUT_glut_LIBRARY})

#### end var setup  ###
SETUP_EXAMPLE(osgbrowser)

