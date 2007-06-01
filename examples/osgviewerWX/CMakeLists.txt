SET(TARGET_SRC osgviewerWX.cpp )

SET(TARGET_H osgviewerWX.h )

INCLUDE(${wxWidgets_USE_FILE})

#### end var setup  ###

SET(REQUIRE_WINMAIN_FLAG ON)
SETUP_EXAMPLE(osgviewerWX)

TARGET_LINK_LIBRARIES("${TARGET_DEFAULT_PREFIX}osgviewerWX" ${wxWidgets_LIBRARIES})
