FILE(GLOB TARGET_SRC *.cpp)
FILE(GLOB TARGET_H *.h)

SET(TARGET_ADDED_LIBRARIES osgParticle )

#### end var setup  ###
SETUP_PLUGIN(osgparticle)
