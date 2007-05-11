SET(TARGET_SRC osgsimpleviewerWX.cpp )

SET(TARGET_H osgsimpleviewerWX.h )
                             
SET(TARGET_ADDED_LIBRARIES ${wxWidgets_LIBRARIES} )

INCLUDE_DIRECTORIES(${wxWidgets_INCLUDE_DIRS} )

ADD_DEFINITIONS( ${wxWidgets_DEFINITIONS} )


#### end var setup  ###
SETUP_EXAMPLE(osgsimpleviewerWX)
