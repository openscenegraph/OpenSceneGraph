FILE(GLOB TARGET_SRC *.cpp)
FILE(GLOB TARGET_H *.h)

SET(TARGET_ADDED_LIBRARIES osgSim )

#### end var setup  ###
SETUP_PLUGIN(osgsim)
