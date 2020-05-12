#use pkg-config to find various modues
find_package(PkgConfig QUIET)

IF(PKG_CONFIG_FOUND)
    #Version 2.35 introduces the rsvg_cleanup function which is used
    PKG_CHECK_MODULES(RSVG librsvg-2.0>=2.35)

    PKG_CHECK_MODULES(CAIRO cairo)

    IF (RSVG_FOUND AND NOT CAIRO_FOUND)
       SET(RSVG_FOUND FALSE)
    ENDIF()

ENDIF()
