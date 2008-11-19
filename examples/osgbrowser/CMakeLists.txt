SET(TARGET_SRC
    llembeddedbrowser.cpp
    llembeddedbrowserwindow.cpp
    llmozlib2.cpp
    nsProfileDirServiceProvider.cpp
    Browser.cpp
    UBrowser.cpp
    osgbrowser.cpp
    ReaderWriterUBrowser.cpp
)

SET(TARGET_H
    llembeddedbrowser.h
    llembeddedbrowserwindow.h
    llmozlib2.h
    nsProfileDirServiceProvider.h
    nsProfileLock.h
    nsProfileStringTypes.h
    Browser.h
    UBrowser.h
)

ADD_DEFINITIONS(-DMOZILLA_INTERNAL_API)

SET(ALL_REQUIRED_XUL_INCLUDE_DIRS
    ${XUL_INCLUDE_DIRS} 
    ${XUL_INCLUDE_DIR}/xpcom
    ${XUL_INCLUDE_DIR}/string 
    ${XUL_INCLUDE_DIR}/locale 
    ${XUL_INCLUDE_DIR}/view 
    ${XUL_INCLUDE_DIR}/content 
    ${XUL_INCLUDE_DIR}/gfx 
    ${XUL_INCLUDE_DIR}/layout 
    ${XUL_INCLUDE_DIR}/webbrwsr 
    ${XUL_INCLUDE_DIR}/widget  
    ${XUL_INCLUDE_DIR}/dom 
    ${XUL_INCLUDE_DIR}/docshell 
    ${XUL_INCLUDE_DIR}/pref 
    ${XUL_INCLUDE_DIR}/nkcache 
    ${XUL_INCLUDE_DIR}/necko/ 
    ${XUL_INCLUDE_DIR}/profdirserviceprovider 
    ${XUL_INCLUDE_DIR}/nkcache 
    ${XUL_INCLUDE_DIR}/xulapp
    ${XUL_INCLUDE_DIR}/profile 
    ${XUL_INCLUDE_DIR}/appshell
    ${XUL_INCLUDE_DIR}/xpcom_obsolete
    ${XUL_INCLUDE_DIR}/intl
    ${XUL_INCLUDE_DIR}/windowwatcher
    ${XUL_INCLUDE_DIR}/xpconnect
)

IF  (WIN32 OR APPLE)

    INCLUDE_DIRECTORIES(${ALL_REQUIRED_XUL_INCLUDE_DIRS} )
    SET(TARGET_EXTERNAL_LIBRARIES ${XUL_LIBRARIES})

ELSE(WIN32 OR APPLE) 

    INCLUDE_DIRECTORIES(${GTK_INCLUDE_DIRS} ${ALL_REQUIRED_XUL_INCLUDE_DIRS} )
    SET(TARGET_EXTERNAL_LIBRARIES ${GTK_LIBRARIES} ${XUL_LIBRARIES})

ENDIF(WIN32 OR APPLE)

ADD_DEFINITIONS(-DXUL_DIR=${XUL_DIR})

#### end var setup  ###
SETUP_EXAMPLE(osgbrowser)

