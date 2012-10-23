SET(TARGET_EXTERNAL_LIBRARIES ${SDL_LIBRARY} )
INCLUDE_DIRECTORIES(${SDL_INCLUDE_DIR} )

SET(TARGET_SRC
    JoystickDevice.cpp
    ReaderWriterSDL.cpp
)

SET(TARGET_H 
    JoystickDevice.h
)

SET(TARGET_ADDED_LIBRARIES osgGA)

#### end var setup  ###
SETUP_PLUGIN(sdl sdl)
