#use pkg-config to find various modues
INCLUDE(FindPkgConfig OPTIONAL)

IF(PKG_CONFIG_FOUND)

    INCLUDE(FindPkgConfig)

    PKG_CHECK_MODULES(GTK gtk+-2.0)

    IF(WIN32)
        PKG_CHECK_MODULES(GTKGL gtkglext-win32-1.0)
    ELSE()
        PKG_CHECK_MODULES(GTKGL gtkglext-x11-1.0)
    ENDIF()

ENDIF()
