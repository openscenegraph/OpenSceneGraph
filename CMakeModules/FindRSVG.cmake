#use pkg-config to find various modues
INCLUDE(FindPkgConfig OPTIONAL)

IF(PKG_CONFIG_FOUND)

    INCLUDE(FindPkgConfig)

    PKG_CHECK_MODULES(RSVG librsvg-2.0)
    PKG_CHECK_MODULES(CAIRO cairo)

    IF (RSVG_FOUND AND NOT CAIRO_FOUND)
       SET(RSVG_FOUND FALSE)
    ENDIF()


ENDIF()
