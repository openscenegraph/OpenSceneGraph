FILE(GLOB TARGET_SRC *.cpp)
FILE(GLOB TARGET_H *.h)

SET(TARGET_ADDED_LIBRARIES osgAnimation )

#### end var setup  ###
SETUP_PLUGIN(osganimation)
