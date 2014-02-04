SET(TARGET_H
    LuaScriptEngine.h
)
SET(TARGET_SRC
    LuaScriptEngine.cpp
    ReaderWriterLua.cpp
)

IF (OSG_USE_LOCAL_LUA_SOURCE)

  IF( CMAKE_SYSTEM MATCHES "Linux" )
    ADD_DEFINITIONS(-DLUA_USE_LINUX)
  ELSEIF(APPLE)
    ADD_DEFINITIONS(-DLUA_USE_MACOSX)
  ENDIF()

  SET(LUA_SRC_DIR lua-5.2.3/src)
  SET(LUA_INCLUDE_DIR ${LUA_SRC_DIR})

  SET(LUA_CORE_SRC
    ${LUA_SRC_DIR}/lapi.c
    ${LUA_SRC_DIR}/lcode.c
    ${LUA_SRC_DIR}/lctype.c
    ${LUA_SRC_DIR}/ldebug.c
    ${LUA_SRC_DIR}/ldo.c
    ${LUA_SRC_DIR}/ldump.c
    ${LUA_SRC_DIR}/lfunc.c
    ${LUA_SRC_DIR}/lgc.c
    ${LUA_SRC_DIR}/llex.c
    ${LUA_SRC_DIR}/lmem.c
    ${LUA_SRC_DIR}/lobject.c
    ${LUA_SRC_DIR}/lopcodes.c
    ${LUA_SRC_DIR}/lparser.c
    ${LUA_SRC_DIR}/lstate.c
    ${LUA_SRC_DIR}/lstring.c
    ${LUA_SRC_DIR}/ltable.c
    ${LUA_SRC_DIR}/ltm.c
    ${LUA_SRC_DIR}/lundump.c
    ${LUA_SRC_DIR}/lvm.c
    ${LUA_SRC_DIR}/lzio.c
  )

  SET(LUA_LIB_SRC
    ${LUA_SRC_DIR}/lauxlib.c
    ${LUA_SRC_DIR}/lbaselib.c
    ${LUA_SRC_DIR}/lbitlib.c
    ${LUA_SRC_DIR}/lcorolib.c
    ${LUA_SRC_DIR}/ldblib.c
    ${LUA_SRC_DIR}/liolib.c
    ${LUA_SRC_DIR}/lmathlib.c
    ${LUA_SRC_DIR}/loslib.c
    ${LUA_SRC_DIR}/lstrlib.c
    ${LUA_SRC_DIR}/ltablib.c
    ${LUA_SRC_DIR}/loadlib.c
    ${LUA_SRC_DIR}/linit.c
  )

  SET(TARGET_SRC ${TARGET_SRC} ${LUA_CORE_SRC} ${LUA_LIB_SRC})
  SET(TARGET_H ${TARGET_H} ${LUA_H})
  INCLUDE_DIRECTORIES(${LUA_INCLUDE_DIR} )

ELSE()
  SET(TARGET_EXTERNAL_LIBRARIES ${TARGET_EXTERNAL_LIBRARIES} ${LUA_LIBRARY})
  INCLUDE_DIRECTORIES(${LUA_INCLUDE_DIR} )
ENDIF()


#### end var setup  ###
SETUP_PLUGIN(lua)
