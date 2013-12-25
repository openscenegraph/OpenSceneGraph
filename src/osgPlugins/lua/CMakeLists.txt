SET(TARGET_H
    MethodObject.h
    LuaScriptEngine.h
)
SET(TARGET_SRC
    MethodObject.cpp
    GeodeMethods.cpp
    GroupMethods.cpp
    LuaScriptEngine.cpp
    ReaderWriterLua.cpp
)

SET(TARGET_EXTERNAL_LIBRARIES ${TARGET_EXTERNAL_LIBRARIES} ${LUA_LIBRARY})
INCLUDE_DIRECTORIES(${LUA_INCLUDE_DIR} )

#### end var setup  ###
SETUP_PLUGIN(lua)
