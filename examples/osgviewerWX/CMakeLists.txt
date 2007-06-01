SET(TARGET_SRC osgsimpleviewerWX.cpp )

SET(TARGET_H osgsimpleviewerWX.h )

INCLUDE(${wxWidgets_USE_FILE})

#### end var setup  ###

SET(REQUIRE_WINMAIN_FLAG ON)
SETUP_EXAMPLE(osgsimpleviewerWX)

TARGET_LINK_LIBRARIES("${TARGET_DEFAULT_PREFIX}osgsimpleviewerWX" ${wxWidgets_LIBRARIES})
