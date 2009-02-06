# Locate XUL
# This module defines
# XUL_LIBRARIES
# XUL_FOUND, if false, do not try to link to gdal
# XUL_INCLUDE_DIR, where to find the headers
#
# $XUL_DIR is an environment variable that would
# correspond to the ./configure --prefix=$XUL_DIR
#
# Created by Robert Osfield.

#use pkg-config to find various modues
INCLUDE(FindPkgConfig OPTIONAL)

IF(PKG_CONFIG_FOUND)

    INCLUDE(FindPkgConfig)

    pkg_check_modules(XULRUNNER_XPCOM xulrunner-xpcom<=1.8.9)
    pkg_check_modules(XULRUNNER_JS xulrunner-js)
    pkg_check_modules(XULRUNNER_NSPR xulrunner-nspr)
    pkg_check_modules(XULRUNNER_NSS xulrunner-nss)

ENDIF(PKG_CONFIG_FOUND)

# Added check to make sure that nsIBaseWindow.h is available, as it's not a standard part the of 1.8.x SDK
FIND_PATH(NSIBASEWINDOW_INCLUDE_DIR widget/nsIBaseWindow.h
    PATHS ${XULRUNNER_XPCOM_INCLUDE_DIRS}
    $ENV{OSG_DIR}/include/xulrunner
    $ENV{OSG_DIR}/include
    $ENV{OSG_DIR}/xulrunner
    $ENV{OSG_DIR}
    $ENV{OSGDIR}/include/xulrunner
    $ENV{OSGDIR}/include
    $ENV{OSGDIR}/xulrunner
    $ENV{OSGDIR}
    $ENV{OSG_ROOT}/include/xulrunner
    $ENV{OSG_ROOT}/include
    ~/Library/Frameworks
    /Library/Frameworks
    /usr/local/include/xulrunner
    /usr/local/include
    /usr/include/xulrunner
    /usr/include
    /sw/include/xulrunner # Fink
    /sw/include # Fink
    /opt/local/include/xulrunner # DarwinPorts
    /opt/local/include # DarwinPorts
    /opt/csw/include/xulrunner # Blastwave
    /opt/csw/include # Blastwave
    /opt/include/xulrunner
    /opt/include
    [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;OSG_ROOT]/include/xulrunner
    [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;OSG_ROOT]/include
    /usr/freeware/include/xulrunner
    /usr/freeware/include
)

FIND_PATH(XUL_INCLUDE_DIR nsEmbedAPI.h
    PATHS ${XULRUNNER_XPCOM_INCLUDE_DIRS}
    $ENV{OSG_DIR}/include/xulrunner
    $ENV{OSG_DIR}/include
    $ENV{OSG_DIR}/xulrunner
    $ENV{OSG_DIR}
    $ENV{OSGDIR}/include/xulrunner
    $ENV{OSGDIR}/include
    $ENV{OSGDIR}/xulrunner
    $ENV{OSGDIR}
    $ENV{OSG_ROOT}/include/xulrunner
    $ENV{OSG_ROOT}/include
    ~/Library/Frameworks
    /Library/Frameworks
    /usr/local/include/xulrunner
    /usr/local/include
    /usr/include/xulrunner
    /usr/include
    /sw/include/xulrunner # Fink
    /sw/include # Fink
    /opt/local/include/xulrunner # DarwinPorts
    /opt/local/include # DarwinPorts
    /opt/csw/include/xulrunner # Blastwave
    /opt/csw/include # Blastwave
    /opt/include/xulrunner
    /opt/include
    [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;OSG_ROOT]/include/xulrunner
    [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;OSG_ROOT]/include
    /usr/freeware/include/xulrunner
    /usr/freeware/include
)

FIND_PATH(NSPR_INCLUDE_DIR prtypes.h
    PATHS ${XULRUNNER_NSPR_INCLUDE_DIRS}
    $ENV{OSG_DIR}/include/nspr
    $ENV{OSG_DIR}/include
    $ENV{OSG_DIR}/nspr
    $ENV{OSG_DIR}
    $ENV{OSGDIR}/include/nspr
    $ENV{OSGDIR}/include
    $ENV{OSGDIR}/nspr
    $ENV{OSGDIR}
    $ENV{OSG_ROOT}/include/nspr
    $ENV{OSG_ROOT}/include
    ~/Library/Frameworks
    /Library/Frameworks
    /usr/local/include/nspr
    /usr/local/include
    /usr/include/nspr
    /usr/include
    /sw/include/nspr # Fink
    /sw/include # Fink
    /opt/local/include/nspr # DarwinPorts
    /opt/local/include # DarwinPorts
    /opt/csw/include/nspr # Blastwave
    /opt/csw/include # Blastwave
    /opt/include/nspr
    /opt/include
    [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;OSG_ROOT]/include/nspr
    [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;OSG_ROOT]/include
    /usr/freeware/include/nspr
    /usr/freeware/include
)

FIND_PATH(MOZJS_INCLUDE_DIR jsapi.h
    PATHS ${XULRUNNER_JS_INCLUDE_DIRS}
    $ENV{OSG_DIR}/include/mozjs
    $ENV{OSG_DIR}/include
    $ENV{OSG_DIR}/mozjs
    $ENV{OSG_DIR}
    $ENV{OSGDIR}/include/mozjs
    $ENV{OSGDIR}/include
    $ENV{OSGDIR}/mozjs
    $ENV{OSGDIR}
    $ENV{OSG_ROOT}/include/mozjs
    $ENV{OSG_ROOT}/include
    ~/Library/Frameworks
    /Library/Frameworks
    /usr/local/include/mozjs
    /usr/local/include
    /usr/include/mozjs
    /usr/include
    /sw/include/mozjs # Fink
    /sw/include # Fink
    /opt/local/include/mozjs # DarwinPorts
    /opt/local/include # DarwinPorts
    /opt/csw/include/mozjs # Blastwave
    /opt/csw/include # Blastwave
    /opt/include/mozjs
    /opt/include
    [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;OSG_ROOT]/include/mozjs
    [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;OSG_ROOT]/include
    /usr/freeware/include/mozjs
    /usr/freeware/include
)

FIND_PATH(XUL_DIR
    NAMES components/appshell.xpt
    PATHS
    $ENV{OSG_DIR}/lib
    $ENV{OSG_DIR}
    $ENV{OSGDIR}/lib
    $ENV{OSGDIR}
    $ENV{OSG_ROOT}/lib
    ~/Library/Frameworks
    /Library/Frameworks
    /usr/local/lib/xulrunner
    /usr/local/lib
    /usr/lib/xulrunner
    /usr/lib
    /sw/lib/xulrunner
    /sw/lib
    /opt/local/lib/xulrunner
    /opt/local/lib
    /opt/csw/lib/xulrunner
    /opt/csw/lib
    /opt/lib/xulrunner
    /opt/lib
    [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;OSG_ROOT]/lib
    /usr/freeware/lib64/xulrunner
    /usr/freeware/lib64
)

MACRO(FIND_XUL_LIBRARY MYLIBRARY MYLIBRARYNAME)

    FIND_LIBRARY(${MYLIBRARY}
        PATHS ${XULRUNNER_XPCOM_LIBRARY_DIRS}
        PATHS ${XULRUNNER_JS_LIBRARY_DIRS}
        PATHS ${XULRUNNER_NSPR_LIBRARY_DIRS}
        PATHS ${XULRUNNER_NSS_LIBRARY_DIRS}
        NAMES ${MYLIBRARYNAME}
        PATHS
        $ENV{OSG_DIR}/lib
        $ENV{OSG_DIR}
        $ENV{OSGDIR}/lib
        $ENV{OSGDIR}
        $ENV{OSG_ROOT}/lib
        ~/Library/Frameworks
        /Library/Frameworks
        /usr/local/lib/xulrunner
        /usr/local/lib
        /usr/lib/xulrunner
        /usr/lib
        /sw/lib/xulrunner
        /sw/lib
        /opt/local/lib/xulrunner
        /opt/local/lib
        /opt/csw/lib/xulrunner
        /opt/csw/lib
        /opt/lib/xulrunner
        /opt/lib
        [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;OSG_ROOT]/lib
        /usr/freeware/lib64/xulrunner
        /usr/freeware/lib64
    )

ENDMACRO(FIND_XUL_LIBRARY LIBRARY LIBRARYNAME)


FIND_XUL_LIBRARY(XUL_LIBRARY xul plds4 plc4 nspr4)
FIND_XUL_LIBRARY(XUL_MOZJS_LIBRARY mozjs)
FIND_XUL_LIBRARY(XUL_XPCOM_LIBRARY xpcom)
FIND_XUL_LIBRARY(XUL_PLUGIN_LIBRARY plds4)
FIND_XUL_LIBRARY(XUL_NSS_LIBRARY nss3)


SET(XUL_FOUND "NO")
IF(XUL_LIBRARY AND XUL_INCLUDE_DIR AND NSIBASEWINDOW_INCLUDE_DIR)

    SET(XUL_FOUND "YES")
    SET(XUL_LIBRARIES ${XUL_LIBRARY} ${XUL_MOZJS_LIBRARY} ${XUL_XPCOM_LIBRARY} ${XUL_PLUGIN_LIBRARY} ${XUL_NSS_LIBRARY})
    SET(XUL_INCLUDE_DIRS ${XUL_INCLUDE_DIR} ${NSPR_INCLUDE_DIR} ${MOZJS_INCLUDE_DIR})

ENDIF(XUL_LIBRARY AND XUL_INCLUDE_DIR AND NSIBASEWINDOW_INCLUDE_DIR)

# MESSAGE("XUL_INCLUDE_DIR " ${XUL_INCLUDE_DIR})
# MESSAGE("XUL_LIBRARIES " ${XUL_LIBRARIES})
