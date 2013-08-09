/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2013 Robert Osfield
 *
 * This library is open source and may be redistributed and/or modified under
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * OpenSceneGraph Public License for more details.
*/

#include "LuaScriptEngine.h"

using namespace lua;

LuaScriptEngine::LuaScriptEngine():
    osg::ScriptEngine("lua"),
    _lua(0)
{
    initialize();
}

LuaScriptEngine::LuaScriptEngine(const LuaScriptEngine& rhs, const osg::CopyOp&):
    osg::ScriptEngine("lua"),
    _lua(0)
{
    initialize();
}

LuaScriptEngine::~LuaScriptEngine()
{
    lua_close(_lua);
}

void LuaScriptEngine::initialize()
{
    _lua = lua_open();
    luaL_openlibs(_lua);
}

void LuaScriptEngine::run(osg::Script* script)
{
    if (!script || !_lua) return;

    if (luaL_dostring(_lua, script->getScript().c_str()))
    {
        OSG_NOTICE << "LuaScriptEngine::run(Script*) error: " << lua_tostring(_lua, -1) << std::endl;
    }
}
