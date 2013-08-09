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

#ifndef LUASCRIPTENGINE_H
#define LUASCRIPTENGINE_H

#include <osg/ScriptEngine>

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

namespace lua
{

class LuaScriptEngine : public osg::ScriptEngine
{
    public:
        LuaScriptEngine();
        LuaScriptEngine(const LuaScriptEngine& rhs, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);

        META_Object(lua, LuaScriptEngine)

        virtual const std::string& getLanguage() const { return _language; }

        /** run a Script.*/
        virtual void run(osg::Script* script);

        /** get the lua_State object.*/
        lua_State* getLuaState() { return _lua; }

    protected:

        void initialize();

        virtual ~LuaScriptEngine();

        lua_State* _lua;
};


}

#endif