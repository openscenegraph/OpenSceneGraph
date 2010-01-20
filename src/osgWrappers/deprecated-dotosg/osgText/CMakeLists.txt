FILE(GLOB TARGET_SRC *.cpp)
FILE(GLOB TARGET_H *.h)

SET(TARGET_ADDED_LIBRARIES osgText )

#### end var setup  ###
SETUP_PLUGIN(osgtext)
