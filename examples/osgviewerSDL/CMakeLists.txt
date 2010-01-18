SET(TARGET_EXTERNAL_LIBRARIES ${SDL_LIBRARY} )

IF("${SDL_LIBRARY}" MATCHES "SDLmain")
       # Found a precompiled SDLmain library.
       SET(TARGET_SRC osgviewerSDL.cpp)
ELSE()
       # No SDLmain has been found.
       # Depending on platform, code may need to be provided for SDLmain.
       IF(APPLE)
               SET(TARGET_SRC osgviewerSDL.cpp SDLMainForMacOSX.m SDLMainForMacOSX.h)
       ELSE()
               # Unhandled case. No libsdlmain has been found and no code substitute.
               # Depending on platform (or SDL version),
               # this may or may not link successfully.
               SET(TARGET_SRC osgviewerSDL.cpp)
               SET(TARGET_EXTERNAL_LIBRARIES ${SDLMAIN_LIBRARY} ${TARGET_EXTERNAL_LIBRARIES} )
       ENDIF()
ENDIF()

IF (MINGW)
    SET(TARGET_EXTERNAL_LIBRARIES mingw32 ${TARGET_EXTERNAL_LIBRARIES} winmm dinput ddraw dxguid)
ENDIF()

INCLUDE_DIRECTORIES(${SDL_INCLUDE_DIR} )

#### end var setup  ###
SETUP_EXAMPLE(osgviewerSDL)

