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
#include <osg/io_utils>

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

bool LuaScriptEngine::loadScript(osg::Script* script)
{
    if (_loadedScripts.count(script)!=0) return true;

    int loadResult = luaL_loadstring(_lua, script->getScript().c_str());
    if (loadResult==0)
    {
        OSG_NOTICE<<"Loaded script"<<std::endl;

        _loadedScripts.insert(script);
        return true;
    }
    else
    {
        OSG_NOTICE << "LuaScriptEngine::luaL_loadstring(Script*) error: " << lua_tostring(_lua, -1) << std::endl;
        return false;
    }
}


bool LuaScriptEngine::run(osg::Script* script, const std::string& entryPoint, Parameters& inputParameters, Parameters& outputParameters)
{
    if (!script || !_lua) return false;

    if (_loadedScripts.count(script)==0)
    {
        if (!loadScript(script)) return false;

        if (lua_pcall(_lua, 0, 0, 0)!=0)
        {
            OSG_NOTICE<< "error initialize script "<< lua_tostring(_lua, -1)<<std::endl;
            return false;
        }
    }

    if (entryPoint.empty())
    {
        int callResult = lua_pcall(_lua, 0, LUA_MULTRET, 0);
        if (callResult)
        {
            OSG_NOTICE << "LuaScriptEngine::call(Script*) error: " << lua_tostring(_lua, -1) << std::endl;
            return false;
        }
        OSG_NOTICE << "Successful run " << std::endl;
        return true;
    }
    else
    {


        lua_getfield(_lua, LUA_GLOBALSINDEX, entryPoint.c_str()); /* function to be called */

        for(osg::ScriptEngine::Parameters::const_iterator itr = inputParameters.begin();
            itr != inputParameters.end();
            ++itr)
        {
            pushParameter(itr->get());
        }


        if (lua_pcall(_lua, inputParameters.size(), outputParameters.size(),0)!=0)
        {
            OSG_NOTICE<<"Lua error : "<<lua_tostring(_lua, -1)<<std::endl;
            return false;
        }

        for(osg::ScriptEngine::Parameters::const_iterator itr = outputParameters.begin();
            itr != outputParameters.end();
            ++itr)
        {
            popParameter(itr->get());
        }
        return true;
    }
    return false;
}


class PushStackValueVisitor : public osg::ValueObject::GetValueVisitor
{
public:

    lua_State* _lua;

    PushStackValueVisitor(lua_State* lua) : _lua(lua) {}

    inline void push(const char* str, double value)
    {
        lua_pushstring(_lua, str); lua_pushnumber(_lua, value); lua_settable(_lua, -3);
    }

    inline void pushElem(unsigned int i, double value)
    {
        lua_pushnumber(_lua, i); lua_pushinteger(_lua, value); lua_settable(_lua, -3);
    }

    virtual void apply(bool value)                      { lua_pushboolean(_lua, value ? 0 : 1); }
    virtual void apply(char value)                      { lua_pushnumber(_lua, value); }
    virtual void apply(unsigned char value)             { lua_pushnumber(_lua, value); }
    virtual void apply(short value)                     { lua_pushnumber(_lua, value); }
    virtual void apply(unsigned short value)            { lua_pushnumber(_lua, value); }
    virtual void apply(int value)                       { lua_pushnumber(_lua, value); }
    virtual void apply(unsigned int value)              { lua_pushnumber(_lua, value); }
    virtual void apply(float value)                     { lua_pushnumber(_lua, value); }
    virtual void apply(double value)                    { lua_pushnumber(_lua, value); }
    virtual void apply(const std::string& value)        { lua_pushlstring(_lua, &value[0], value.size()); }
    virtual void apply(const osg::Vec2f& value)         { lua_newtable(_lua); push("x", value.x()); push("y", value.y()); }
    virtual void apply(const osg::Vec3f& value)         { lua_newtable(_lua); push("x", value.x()); push("y", value.y()); push("z", value.z()); }
    virtual void apply(const osg::Vec4f& value)         { lua_newtable(_lua); push("x", value.x()); push("y", value.y()); push("z", value.z());  push("w", value.w()); }
    virtual void apply(const osg::Vec2d& value)         { lua_newtable(_lua); push("x", value.x()); push("y", value.y()); }
    virtual void apply(const osg::Vec3d& value)         { lua_newtable(_lua); push("x", value.x()); push("y", value.y()); push("z", value.z()); }
    virtual void apply(const osg::Vec4d& value)         { lua_newtable(_lua); push("x", value.x()); push("y", value.y()); push("z", value.z());  push("w", value.w()); }
    virtual void apply(const osg::Quat& value)          { lua_newtable(_lua); push("x", value.x()); push("y", value.y()); push("z", value.z());  push("w", value.w()); }
    virtual void apply(const osg::Plane& value)         { lua_newtable(_lua); pushElem(0, value[0]); pushElem(1, value[1]); pushElem(2, value[2]);  pushElem(3, value[3]); }
    virtual void apply(const osg::Matrixf& value)       { lua_newtable(_lua); for(unsigned int r=0; r<4; ++r) { for(unsigned int c=0; c<4; ++c) { pushElem(r*4+c, value(r,c)); } } }
    virtual void apply(const osg::Matrixd& value)       { lua_newtable(_lua); for(unsigned int r=0; r<4; ++r) { for(unsigned int c=0; c<4; ++c) { pushElem(r*4+c, value(r,c)); } } }
};

class GetStackValueVisitor : public osg::ValueObject::SetValueVisitor
{
public:

    lua_State* _lua;
    int _index;
    int _numberToPop;

    GetStackValueVisitor(lua_State* lua, int index) : _lua(lua), _index(index), _numberToPop(0) {}

    void print(int index)
    {
        OSG_NOTICE<<"lua_type("<<index<<") = ";
        switch(lua_type(_lua, index))
        {
            case(LUA_TNIL): OSG_NOTICE<<"LUA_TNIL "<<std::endl; break;
            case(LUA_TNUMBER): OSG_NOTICE<<"LUA_TNUMBER "<<lua_tonumber(_lua, index)<<std::endl; break;
            case(LUA_TBOOLEAN): OSG_NOTICE<<"LUA_TBOOLEAN "<<lua_toboolean(_lua, index)<<std::endl; break;
            case(LUA_TSTRING): OSG_NOTICE<<"LUA_TSTRING "<<lua_tostring(_lua, index)<<std::endl; break;
            case(LUA_TTABLE): OSG_NOTICE<<"LUA_TTABLE "<<std::endl; break;
            case(LUA_TFUNCTION): OSG_NOTICE<<"LUA_TFUNCTION "<<std::endl; break;
            case(LUA_TUSERDATA): OSG_NOTICE<<"LUA_TUSERDATA "<<std::endl; break;
            case(LUA_TTHREAD): OSG_NOTICE<<"LUA_TTHREAD "<<std::endl; break;
            case(LUA_TLIGHTUSERDATA): OSG_NOTICE<<"LUA_TLIGHTUSERDATA "<<std::endl; break;
            default: OSG_NOTICE<<lua_typename(_lua, index)<<std::endl; break;
        }
    }

    template<typename T>
    void get2(T& value)
    {
        if (lua_istable(_lua, _index))
        {
            lua_getfield(_lua, _index,   "x");
            lua_getfield(_lua, _index-1, "y");

            if (lua_isnumber(_lua, -2)) value.x() = lua_tonumber(_lua, -2);
            if (lua_isnumber(_lua, -1)) value.y() = lua_tonumber(_lua, -1);

            _numberToPop = 3;
        }
    }

    template<typename T>
    void get3(T& value)
    {
        if (lua_istable(_lua, _index))
        {
            lua_getfield(_lua, _index,   "x");
            lua_getfield(_lua, _index-1, "y");
            lua_getfield(_lua, _index-2, "z");

            if (lua_isnumber(_lua, -3)) value.x() = lua_tonumber(_lua, -3);
            if (lua_isnumber(_lua, -2)) value.y() = lua_tonumber(_lua, -2);
            if (lua_isnumber(_lua, -1)) value.z() = lua_tonumber(_lua, -1);

            _numberToPop = 4;
        }
    }

    template<typename T>
    void get4(T& value)
    {
        if (lua_istable(_lua, _index))
        {
            lua_getfield(_lua, _index,   "x");
            lua_getfield(_lua, _index-1, "y");
            lua_getfield(_lua, _index-2, "z");
            lua_getfield(_lua, _index-3, "w");

            if (lua_isnumber(_lua, -4)) value.x() = lua_tonumber(_lua, -4);
            if (lua_isnumber(_lua, -3)) value.y() = lua_tonumber(_lua, -3);
            if (lua_isnumber(_lua, -2)) value.z() = lua_tonumber(_lua, -2);
            if (lua_isnumber(_lua, -1)) value.w() = lua_tonumber(_lua, -1);

            _numberToPop = 5;
        }
    }

    template<typename T>
    void getMatrix(T& value)
    {
        if (lua_istable(_lua, _index))
        {
            for(unsigned int r=0; r<4; ++r)
            {
                for(unsigned c=0; c<4; ++c)
                {
                    lua_rawgeti(_lua, _index, r*4+c);
                    if (lua_isnumber(_lua, -1)) value(r,c) = lua_tonumber(_lua, -1);
                    lua_pop(_lua, 1);
                }
            }

            _numberToPop = 1;
        }
    }

    virtual void apply(bool& value)             { if (lua_isboolean(_lua, _index)) { value = (lua_toboolean(_lua, _index)!=0); _numberToPop = 1; } }
    virtual void apply(char& value)             { if (lua_isnumber(_lua, _index)) { value = lua_tonumber(_lua, _index)!=0; _numberToPop = 1; } }
    virtual void apply(unsigned char& value)    { if (lua_isnumber(_lua, _index)) { value = lua_tonumber(_lua, _index)!=0; _numberToPop = 1; } }
    virtual void apply(short& value)            { if (lua_isnumber(_lua, _index)) { value = lua_tonumber(_lua, _index)!=0; _numberToPop = 1; } }
    virtual void apply(unsigned short& value)   { if (lua_isnumber(_lua, _index)) { value = lua_tonumber(_lua, _index)!=0; _numberToPop = 1; } }
    virtual void apply(int& value)              { if (lua_isnumber(_lua, _index)) { value = lua_tonumber(_lua, _index)!=0; _numberToPop = 1; } }
    virtual void apply(unsigned int& value)     { if (lua_isnumber(_lua, _index)) { value = lua_tonumber(_lua, _index)!=0; _numberToPop = 1; } }
    virtual void apply(float& value)            { if (lua_isnumber(_lua, _index)) { value = lua_tonumber(_lua, _index)!=0; _numberToPop = 1; } }
    virtual void apply(double& value)           { if (lua_isnumber(_lua, _index)) { value = lua_tonumber(_lua, _index)!=0; _numberToPop = 1; } }
    virtual void apply(std::string& value)      { if (lua_isstring(_lua, _index)) { value = std::string(lua_tostring(_lua, _index), lua_strlen(_lua, _index)); } OSG_NOTICE<<"got string value = "<<value<<std::endl; }
    virtual void apply(osg::Vec2f& value)       { get2(value); }
    virtual void apply(osg::Vec3f& value)       { get3(value); }
    virtual void apply(osg::Vec4f& value)       { get4(value); }
    virtual void apply(osg::Vec2d& value)       { get2(value); }
    virtual void apply(osg::Vec3d& value)       { get3(value); }
    virtual void apply(osg::Vec4d& value)       { get4(value); }
    virtual void apply(osg::Quat& value)        { get4(value); }
    virtual void apply(osg::Plane& value)
    {
        if (lua_istable(_lua, _index))
        {
            lua_rawgeti(_lua, _index,   0);
            lua_rawgeti(_lua, _index-1, 1);
            lua_rawgeti(_lua, _index-2, 2);
            lua_rawgeti(_lua, _index-3, 3);

            if (lua_isnumber(_lua, -4)) value[0] = lua_tonumber(_lua, -4);
            if (lua_isnumber(_lua, -3)) value[1] = lua_tonumber(_lua, -3);
            if (lua_isnumber(_lua, -2)) value[2] = lua_tonumber(_lua, -2);
            if (lua_isnumber(_lua, -1)) value[3] = lua_tonumber(_lua, -1);

            _numberToPop = 5;
        }
    }
    virtual void apply(osg::Matrixf& value) { getMatrix(value); }
    virtual void apply(osg::Matrixd& value) { getMatrix(value); }
};


bool LuaScriptEngine::pushParameter(osg::Object* object)
{
    OSG_NOTICE<<"pushParameter("<<object->className()<<")"<<std::endl;

    osg::ValueObject* vo = dynamic_cast<osg::ValueObject*>(object);
    if (vo)
    {
        PushStackValueVisitor pvv(_lua);
        vo->get(pvv);
    }
    else
    {
        lua_pushstring(_lua, object->className());
    }

    return false;
}

bool LuaScriptEngine::popParameter(osg::Object* object)
{
    OSG_NOTICE<<"popParameter("<<object->className()<<")"<<std::endl;

    osg::ValueObject* vo = dynamic_cast<osg::ValueObject*>(object);
    if (vo)
    {
        GetStackValueVisitor pvv(_lua, -1);
        vo->set(pvv);
        lua_pop(_lua, pvv._numberToPop);
    }
    else
    {
        if (lua_isstring(_lua, -1)) { OSG_NOTICE<<"popParameter() string = "<<lua_tostring(_lua, -1)<<std::endl; }
        else { OSG_NOTICE<<"popParameter() lua_type = "<<lua_type(_lua, -1)<<std::endl; }
        lua_pop(_lua, 1);
    }


    return false;
}
