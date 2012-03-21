// -*-c++-*- osgWidget - Code by: Jeremy Moles (cubicool) 2007-2008

#include <osgDB/FileUtils>
#include <osgWidget/Lua>
#include <osgWidget/Box>
#include <osgWidget/WindowManager>

// If you want to build with LUA, include it--otherwise, typedef some of the data types
// so that we don't pollute our code too much with conditional includes.
#ifdef OSGWIDGET_USELUA
extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}
#endif

namespace osgWidget {

// This namespace will include all of our Lua library functions. Otherwise, it'll be
// an empty namespace. This is 100% for code clarity, as this namespace is internal and
// not visible to the outside C/C++ world.
namespace lua {
#ifdef OSGWIDGET_USELUA

// Strings representing our global REGISTRY values.
const char* G_WM = "osgWidget_G_WindowManager";

WindowManager* getWindowManager(lua_State* L) {
    lua_pushstring(L, G_WM);
    lua_gettable(L, LUA_REGISTRYINDEX);

    return reinterpret_cast<WindowManager*>(lua_touserdata(L, -1));
}

int newWindow(lua_State* L) {
    osg::ref_ptr<Window> w = new Box("testLUA", Box::HORIZONTAL);

    lua_pushstring(L, w->getName().c_str());

    return 1;
}

int newWidget(lua_State* L) {
    osg::ref_ptr<Widget> w = new Widget("testLUA", 0.0f, 0.0f);

    lua_pushstring(L, w->getName().c_str());

    return 1;
}

int getWindow(lua_State* L) {
    WindowManager* wm = getWindowManager(L);

    lua_pushlightuserdata(L, wm);

    return 1;
}

#endif
}

// A helper function for all those cases where we need to inform the user that there isn't
// a LUA engine available.
bool noLuaFail(const std::string& err) {
    warn() << err << "; Lua not compiled in library." << std::endl;

    return false;
}

// Our "private", internal data.
struct LuaEngineData {
#ifdef OSGWIDGET_USELUA
    LuaEngineData():
    lua(0) {
    }

    lua_State* lua;
#endif
};

LuaEngine::LuaEngine(WindowManager* wm):
_wm(wm) {
#ifdef OSGWIDGET_USELUA
    _data = new LuaEngineData();

#else
    _data = 0;
#endif
}

bool LuaEngine::initialize() {
#ifdef OSGWIDGET_USELUA
    _data->lua = lua_open();

    luaL_openlibs(_data->lua);

    static const struct luaL_reg library[] = {
        {"newWindow", lua::newWindow},
        {"newWidget", lua::newWidget},
        {"getWindow", lua::getWindow},
        {0, 0}
    };

    luaL_openlib(_data->lua, "osgwidget", library, 0);

    // An alternative to using the Registry here would be to pass the WindowManager
    // as a userdata "closure" (pvalue). Please see the following doc on more info:
    // http://www.lua.org/pil/27.3.3.html
    lua_pushstring(_data->lua, lua::G_WM);
    lua_pushlightuserdata(_data->lua, _wm);
    lua_settable(_data->lua, LUA_REGISTRYINDEX);

    return true;

#else
    return noLuaFail("Can't initialize the LuaEngine");
#endif
}

bool LuaEngine::close() {
#ifdef OSGWIDGET_USELUA
    lua_close(_data->lua);

    delete _data;

    return true;

#else
    return noLuaFail("Can't close the LuaEngine");
#endif
}

bool LuaEngine::eval(const std::string& code) {
#ifdef OSGWIDGET_USELUA
    if(luaL_dostring(_data->lua, code.c_str())) {
        warn() << "LuaEngine::eval - " << lua_tostring(_data->lua, -1) << std::endl;

        return false;
    }

    return true;

#else
    return noLuaFail("Can't evaluate code in LuaEngine");
#endif
}

bool LuaEngine::runFile(const std::string& filePath) {
#ifdef OSGWIDGET_USELUA
    if(!osgDB::fileExists(filePath)) {
        warn() << "Couldn't find file \"" << filePath << "\" for LuaEngine." << std::endl;

        return false;
    }

    if(luaL_dofile(_data->lua, filePath.c_str())) {
        warn() << "LuaEngine::runFile - " << lua_tostring(_data->lua, -1) << std::endl;

        return false;
    }

    return true;

#else
    return noLuaFail("Can't run file in LuaEngine");
#endif
}

}
