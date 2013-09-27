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

static int getProperty(lua_State * _lua)
{
    const LuaScriptEngine* lse = reinterpret_cast<const LuaScriptEngine*>(lua_topointer(_lua, lua_upvalueindex(1)));

    int n = lua_gettop(_lua);    /* number of arguments */
    if (n==2)
    {
        if (lua_type(_lua, 1)==LUA_TTABLE &&
            lua_type(_lua, 2)==LUA_TSTRING)
        {
            std::string propertyName = lua_tostring(_lua, 2);

            osg::Object* object = 0;
            lua_pushstring(_lua, "object_ptr");
            lua_rawget(_lua, 1);
            if (lua_type(_lua, -1)==LUA_TLIGHTUSERDATA)
            {
                object = const_cast<osg::Object*>(reinterpret_cast<const osg::Object*>(lua_topointer(_lua,-1)));
            }
            lua_pop(_lua,1);

            return lse->pushPropertyToStack(object, propertyName);
        }
    }

    OSG_NOTICE<<"Warning: Lua getProperty() not matched"<<std::endl;
    return 0;
}


static int setProperty(lua_State* _lua)
{
    const LuaScriptEngine* lse = reinterpret_cast<const LuaScriptEngine*>(lua_topointer(_lua, lua_upvalueindex(1)));

    int n = lua_gettop(_lua);    /* number of arguments */
    if (n==3)
    {
        if (lua_type(_lua, 1)==LUA_TTABLE &&
            lua_type(_lua, 2)==LUA_TSTRING)
        {
            std::string propertyName = lua_tostring(_lua, 2);

            osg::Object* object = 0;
            lua_pushstring(_lua, "object_ptr");
            lua_rawget(_lua, 1);
            if (lua_type(_lua, -1)==LUA_TLIGHTUSERDATA)
            {
                object = const_cast<osg::Object*>(reinterpret_cast<const osg::Object*>(lua_topointer(_lua,-1)));
            }
            lua_pop(_lua,1);

            return lse->setPropertyFromStack(object, propertyName);
        }
    }

    OSG_NOTICE<<"Warning: Lua getProperty() not matched"<<std::endl;
    return 0;
}


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

    luaL_newmetatable(_lua, "LuaScriptEngine.Object");

    lua_pushstring(_lua, "__index");
    lua_pushlightuserdata(_lua, this);
    lua_pushcclosure(_lua, getProperty, 1);
    lua_settable(_lua, -3);

    lua_pushstring(_lua, "__newindex");
    lua_pushlightuserdata(_lua, this);
    lua_pushcclosure(_lua, setProperty, 1);
    lua_settable(_lua, -3);
}

bool LuaScriptEngine::loadScript(osg::Script* script)
{
    if (_loadedScripts.count(script)!=0) return true;

    int loadResult = luaL_loadstring(_lua, script->getScript().c_str());
    if (loadResult==0)
    {
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

int LuaScriptEngine::pushPropertyToStack(osg::Object* object, const std::string& propertyName) const
{
    osgDB::BaseSerializer::Type type;
    if (!_pi.getPropertyType(object, propertyName, type))
    {
        OSG_NOTICE<<"LuaScriptEngine::pushPropertyToStack("<<object<<", "<<propertyName<<") no property found."<<std::endl;
        return 0;
    }

    switch(type)
    {
        case(osgDB::BaseSerializer::RW_STRING):
        {
            std::string value;
            if (_pi.getProperty(object, propertyName, value))
            {
                lua_pushstring(_lua, value.c_str());
                return 1;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_ENUM):
        case(osgDB::BaseSerializer::RW_INT):
        {
            int value;
            if (_pi.getProperty(object, propertyName, value))
            {
                lua_pushinteger(_lua, value);
                return 1;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_UINT):
        {
            unsigned int value;
            if (_pi.getProperty(object, propertyName, value))
            {
                lua_pushinteger(_lua, value);
                return 1;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_FLOAT):
        {
            float value;
            if (_pi.getProperty(object, propertyName, value))
            {
                lua_pushnumber(_lua, value);
                return 1;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_DOUBLE):
        {
            double value;
            if (_pi.getProperty(object, propertyName, value))
            {
                lua_pushnumber(_lua, value);
                return 1;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_VEC2F):
        {
            osg::Vec2f value;
            if (_pi.getProperty(object, propertyName, value))
            {
                lua_newtable(_lua);
                lua_pushstring(_lua, "x"); lua_pushnumber(_lua, value.x()); lua_settable(_lua, -3);
                lua_pushstring(_lua, "y"); lua_pushnumber(_lua, value.y()); lua_settable(_lua, -3);
                return 1;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_VEC3F):
        {
            osg::Vec3f value;
            if (_pi.getProperty(object, propertyName, value))
            {
                lua_newtable(_lua);
                lua_pushstring(_lua, "x"); lua_pushnumber(_lua, value.x()); lua_settable(_lua, -3);
                lua_pushstring(_lua, "y"); lua_pushnumber(_lua, value.y()); lua_settable(_lua, -3);
                lua_pushstring(_lua, "z"); lua_pushnumber(_lua, value.z()); lua_settable(_lua, -3);
                return 1;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_VEC4F):
        {
            osg::Vec4f value;
            if (_pi.getProperty(object, propertyName, value))
            {
                lua_newtable(_lua);
                lua_pushstring(_lua, "x"); lua_pushnumber(_lua, value.x()); lua_settable(_lua, -3);
                lua_pushstring(_lua, "y"); lua_pushnumber(_lua, value.y()); lua_settable(_lua, -3);
                lua_pushstring(_lua, "z"); lua_pushnumber(_lua, value.z()); lua_settable(_lua, -3);
                lua_pushstring(_lua, "w"); lua_pushnumber(_lua, value.w()); lua_settable(_lua, -3);
                return 1;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_MATRIX):
        case(osgDB::BaseSerializer::RW_MATRIXD):
        {
            osg::Matrixd value;
            if (_pi.getProperty(object, propertyName, value))
            {
                lua_newtable(_lua);

                for(unsigned int r=0; r<4; ++r)
                {
                    for(unsigned int c=0; c<4; ++c)
                    {
                        lua_pushnumber(_lua, r*4+c); lua_pushinteger(_lua, value(r,c)); lua_settable(_lua, -3);
                    }
                }
                return 1;
            }
            break;
        }
        default:
            break;
    }

    OSG_NOTICE<<"LuaScriptEngine::pushPropertyToStack("<<object<<", "<<propertyName<<") property of type = "<<_pi.getTypeName(type)<<" error, not supported."<<std::endl;
    return 0;
}

int LuaScriptEngine::setPropertyFromStack(osg::Object* object, const std::string& propertyName) const
{
    osgDB::BaseSerializer::Type type;
    if (!_pi.getPropertyType(object, propertyName, type))
    {
        switch(lua_type(_lua, -1))
        {
            case(LUA_TBOOLEAN):
            {
                _pi.setProperty(object, propertyName, static_cast<bool>(lua_toboolean(_lua, -1)!=0));
                return 0;
            }
            case(LUA_TNUMBER):
            {
                _pi.setProperty(object, propertyName, lua_tonumber(_lua, -1));
                return 0;
            }
            case(LUA_TSTRING):
            {
                _pi.setProperty(object, propertyName, std::string(lua_tostring(_lua, -1)));
                return 0;
            }
            case(LUA_TTABLE):
            {
                typedef std::pair<int, int> TypePair;
                typedef std::vector< TypePair > Types;
                Types types;
                int n = lua_gettop(_lua);    /* number of arguments */
                lua_pushnil(_lua);
                while (lua_next(_lua, n) != 0)
                {
                    types.push_back( TypePair( lua_type(_lua, -2), lua_type(_lua, -1) ) );
                    lua_pop(_lua, 1); // remove value, leave key for next iteration
                }

                OSG_NOTICE<<"Number of elements in Table = "<<types.size()<<std::endl;
                for(Types::iterator itr = types.begin();
                    itr != types.end();
                    ++itr)
                {
                    OSG_NOTICE<<"   "<<lua_typename(_lua, itr->first)<<", "<<lua_typename(_lua, itr->second)<<std::endl;
                }

                return 0;
            }
            default:
                OSG_NOTICE<<"LuaScriptEngine::setPropertyFromStack("<<object<<", "<<propertyName<<") no property found."<<std::endl;
                break;
        }
        return 0;
    }

    switch(type)
    {
        case(osgDB::BaseSerializer::RW_BOOL):
        {
            if (lua_isboolean(_lua, -1))
            {
                _pi.setProperty(object, propertyName, static_cast<bool>(lua_toboolean(_lua, -1)!=0));
                return 0;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_STRING):
        {
            if (lua_isstring(_lua, -1))
            {
                _pi.setProperty(object, propertyName, std::string(lua_tostring(_lua, -1)));
                return 0;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_ENUM):
        case(osgDB::BaseSerializer::RW_INT):
        {
            if (lua_isnumber(_lua, -1))
            {
                _pi.setProperty(object, propertyName, static_cast<int>(lua_tonumber(_lua, -1)));
                return 0;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_UINT):
        {
            if (lua_isnumber(_lua, -1))
            {
                _pi.setProperty(object, propertyName, static_cast<unsigned int>(lua_tonumber(_lua, -1)));
                return 0;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_FLOAT):
        {
            if (lua_isnumber(_lua, -1))
            {
                _pi.setProperty(object, propertyName, static_cast<float>(lua_tonumber(_lua, -1)));
                return 0;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_DOUBLE):
        {
            if (lua_isnumber(_lua, -1))
            {
                _pi.setProperty(object, propertyName, static_cast<double>(lua_tonumber(_lua, -1)));
                return 0;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_VEC2F):
        {
            osg::Vec2f value;
            if (getValue(value))
            {
                _pi.setProperty(object, propertyName, value);
                return 0;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_VEC3F):
        {
            osg::Vec3f value;
            if (getValue(value))
            {
                _pi.setProperty(object, propertyName, value);
                return 0;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_VEC4F):
        {
            osg::Vec4f value;
            if (getValue(value))
            {
                _pi.setProperty(object, propertyName, value);
                return 0;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_MATRIXF):
        {
            osg::Matrixd value;
            if (getValue(value))
            {
                _pi.setProperty(object, propertyName, value);
                return 0;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_MATRIX):
        case(osgDB::BaseSerializer::RW_MATRIXD):
        {
            osg::Matrixd value;
            if (getValue(value))
            {
                _pi.setProperty(object, propertyName, value);
                return 0;
            }
            break;
        }
        default:
            break;
    }
    OSG_NOTICE<<"LuaScriptEngine::setPropertyFromStack("<<object<<", "<<propertyName<<") property of type = "<<_pi.getTypeName(type)<<" not implemented"<<std::endl;
    return 0;
}



bool LuaScriptEngine::getfields(const char* f1, const char* f2, int type) const
{
    lua_getfield(_lua, -1, f1);
    if (lua_type(_lua, -1)!=type) { lua_pop(_lua, 1); return false; }

    lua_getfield(_lua, -2, f2);
    if (lua_type(_lua, -1)!=type) { lua_pop(_lua, 2); return false; }

    return true;
}

bool LuaScriptEngine::getfields(const char* f1, const char* f2, const char* f3, int type) const
{
    lua_getfield(_lua, -1, f1);
    if (lua_type(_lua, -1)!=type) { lua_pop(_lua, 1); return false; }

    lua_getfield(_lua, -2, f2);
    if (lua_type(_lua, -1)!=type) { lua_pop(_lua, 2); return false; }

    lua_getfield(_lua, -3, f3);
    if (lua_type(_lua, -1)!=type) { lua_pop(_lua, 3); return false; }

    return true;
}

bool LuaScriptEngine::getfields(const char* f1, const char* f2, const char* f3, const char* f4, int type) const
{
    lua_getfield(_lua, -1, f1);
    if (lua_type(_lua, -1)!=type) { lua_pop(_lua, 1); return false; }

    lua_getfield(_lua, -2, f2);
    if (lua_type(_lua, -1)!=type) { lua_pop(_lua, 2); return false; }

    lua_getfield(_lua, -3, f3);
    if (lua_type(_lua, -1)!=type) { lua_pop(_lua, 3); return false; }

    lua_getfield(_lua, -4, f4);
    if (lua_type(_lua, -1)!=type) { lua_pop(_lua, 4); return false; }

    return true;
}

bool LuaScriptEngine::getelements(int numElements, int type) const
{
    int abs_pos = lua_gettop(_lua);
    for(int i=0; i<numElements; ++i)
    {
        lua_pushinteger(_lua, i);
        lua_gettable(_lua, abs_pos);
        if (lua_type(_lua, -1)!=type) { lua_pop(_lua, i+1); return false; }
    }
    return true;
}


bool LuaScriptEngine::isType(int pos, osgDB::BaseSerializer::Type type) const
{
    return false;
}

osgDB::BaseSerializer::Type LuaScriptEngine::getType(int pos) const
{
    return osgDB::BaseSerializer::RW_UNDEFINED;
}

bool LuaScriptEngine::getValue(osg::Vec2f& value) const
{
    if (lua_istable(_lua, -1))
    {
        if (getfields("x", "y", LUA_TNUMBER) ||
            getfields("s", "t", LUA_TNUMBER) ||
            getelements(2, LUA_TNUMBER))
        {
            value.set(lua_tonumber(_lua, -2), lua_tonumber(_lua, -1));
            lua_pop(_lua, 2);
            return true;
        }
    }
    return false;
}

bool LuaScriptEngine::getValue(osg::Vec3f& value) const
{
    if (lua_istable(_lua, -1))
    {
        if (getfields("x", "y", "z", LUA_TNUMBER) ||
            getfields("r", "g", "b", LUA_TNUMBER) ||
            getfields("red", "green", "blue", LUA_TNUMBER) ||
            getfields("s", "t", "r", LUA_TNUMBER) ||
            getelements(3, LUA_TNUMBER))
        {
            value.set(lua_tonumber(_lua, -3), lua_tonumber(_lua, -2), lua_tonumber(_lua, -1));
            lua_pop(_lua, 3);
            return true;
        }
    }
    return false;
}

bool LuaScriptEngine::getValue(osg::Vec4f& value) const
{
    if (lua_istable(_lua, -1))
    {
        if (getfields("x", "y", "z", LUA_TNUMBER) ||
            getfields("r", "g", "b", "a", LUA_TNUMBER) ||
            getfields("red", "green", "blue", "alpha", LUA_TNUMBER) ||
            getfields("s", "t", "r", "q", LUA_TNUMBER) ||
            getelements(4, LUA_TNUMBER))
        {
            value.set(lua_tonumber(_lua, -4), lua_tonumber(_lua, -3), lua_tonumber(_lua, -2), lua_tonumber(_lua, -1));
            lua_pop(_lua, 4);
            return true;
        }
    }
    return false;
}

bool LuaScriptEngine::getValue(osg::Matrixf& value) const
{
    if (lua_istable(_lua, -1))
    {
        if (getelements(16,LUA_TNUMBER))
        {
            for(int r=0; r<4; ++r)
            {
                for(int c=0; c<4; ++c)
                {
                    value(r,c) = lua_tonumber(_lua, -16+(r*4+c));
                }
            }
            return true;
        }
    }
    return false;
}

bool LuaScriptEngine::getValue(osg::Matrixd& value) const
{
    if (lua_istable(_lua, -1))
    {
        if (getelements(16,LUA_TNUMBER))
        {
            for(int r=0; r<4; ++r)
            {
                for(int c=0; c<4; ++c)
                {
                    value(r,c) = lua_tonumber(_lua, -16+(r*4+c));
                }
            }
            return true;
        }
    }
    return false;
}

void LuaScriptEngine::pushValue(const osg::Vec2f& value) const
{
}

void LuaScriptEngine::pushValue(const osg::Vec3f& value) const
{
}

void LuaScriptEngine::pushValue(const osg::Vec4f& value) const
{
}

void LuaScriptEngine::pushValue(const osg::Matrixf& value) const
{
}

void LuaScriptEngine::pushValue(const osg::Matrixd& value) const
{
}

bool LuaScriptEngine::pushParameter(osg::Object* object)
{
    osg::ValueObject* vo = dynamic_cast<osg::ValueObject*>(object);
    if (vo)
    {
        PushStackValueVisitor pvv(_lua);
        vo->get(pvv);
    }
    else
    {
        osgDB::PropertyInterface::PropertyMap properties;
        lua_newtable(_lua);
        lua_pushstring(_lua, "object_ptr"); lua_pushlightuserdata(_lua, object); lua_settable(_lua, -3);
#if 1
        lua_pushstring(_lua, "libraryName"); lua_pushstring(_lua, object->libraryName()); lua_settable(_lua, -3);
        lua_pushstring(_lua, "className"); lua_pushstring(_lua, object->className()); lua_settable(_lua, -3);
#endif
        luaL_getmetatable(_lua, "LuaScriptEngine.Object");
        lua_setmetatable(_lua, -2);

    }

    return false;
}

bool LuaScriptEngine::popParameter(osg::Object* object)
{
    osg::ValueObject* vo = dynamic_cast<osg::ValueObject*>(object);
    if (vo)
    {
        GetStackValueVisitor pvv(_lua, -1);
        vo->set(pvv);
        lua_pop(_lua, pvv._numberToPop);
    }
    else
    {
        lua_pop(_lua, 1);
    }


    return false;
}
