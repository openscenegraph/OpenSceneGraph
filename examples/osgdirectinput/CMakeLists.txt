SET(TARGET_H DirectInputRegistry)
SET(TARGET_SRC osgdirectinput.cpp DirectInputRegistry.cpp)

INCLUDE_DIRECTORIES( ${DIRECTINPUT_INCLUDE_DIR} )
SET(TARGET_ADDED_LIBRARIES ${DIRECTINPUT_LIBRARIES})

#### end var setup  ###
SETUP_EXAMPLE(osgdirectinput)

