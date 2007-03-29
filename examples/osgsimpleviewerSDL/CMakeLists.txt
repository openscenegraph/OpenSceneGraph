IF("${SDL_LIBRARY}" MATCHES "SDLmain")
       # Found a precompiled SDLmain library.
       SET(TARGET_SRC osgsimpleviewerSDL.cpp)
ELSE("${SDL_LIBRARY}" MATCHES "SDLmain")
       # No SDLmain has been found.
       # Depending on platform, code may need to be provided for SDLmain.
       IF(APPLE)
               SET(TARGET_SRC osgsimpleviewerSDL.cpp SDLMainForMacOSX.m SDLMainForMacOSX.h)
       ELSE(APPLE)
               # Unhandled case. No libsdlmain has been found and no code substitute.
               # Depending on platform (or SDL version),
               # this may or may not link successfully.
               SET(TARGET_SRC osgsimpleviewerSDL.cpp)
       ENDIF(APPLE)
ENDIF("${SDL_LIBRARY}" MATCHES "SDLmain")

SET(TARGET_EXTERNAL_LIBRARIES ${SDL_LIBRARY} )

INCLUDE_DIRECTORIES(${SDL_INCLUDE_DIR} )

#### end var setup  ###
SETUP_EXAMPLE(osgsimpleviewerSDL)

