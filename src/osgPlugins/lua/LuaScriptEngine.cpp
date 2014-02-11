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
#include <osgDB/ReadFile>

using namespace lua;

class LuaCallbackObject : public osg::CallbackObject
{
public:
    LuaCallbackObject(const std::string& methodName, const LuaScriptEngine* lse, int ref):_lse(lse),_ref(ref) { setName(methodName); }

    virtual bool run(osg::Object* object, osg::Parameters& inputParameters, osg::Parameters& outputParameters) const
    {
        int topBeforeCall = lua_gettop(_lse->getLuaState());

        lua_rawgeti(_lse->getLuaState(), LUA_REGISTRYINDEX, _ref);

        int numInputs = 1;
        _lse->pushParameter(object);

        for(osg::Parameters::iterator itr = inputParameters.begin();
            itr != inputParameters.end();
            ++itr)
        {
            _lse->pushParameter(itr->get());
            ++numInputs;
        }

        if (lua_pcall(_lse->getLuaState(), numInputs, LUA_MULTRET,0)!=0)
        {
            OSG_NOTICE<<"Lua error : "<<lua_tostring(_lse->getLuaState(), -1)<<std::endl;
            return false;
        }

        int topAfterCall = lua_gettop(_lse->getLuaState());
        int numReturns = topAfterCall-topBeforeCall;
        for(int i=1; i<=numReturns; ++i)
        {
            outputParameters.insert(outputParameters.begin(), _lse->popParameterObject());
        }
        return true;
    }

    int getRef() const { return _ref; }

protected:

    osg::ref_ptr<const LuaScriptEngine> _lse;
    int _ref;
};


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

            if (lua_type(_lua, -1)==LUA_TUSERDATA)
            {
                object = *const_cast<osg::Object**>(reinterpret_cast<const osg::Object**>(lua_touserdata(_lua,-1)));
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

            if (lua_type(_lua, -1)==LUA_TUSERDATA)
            {
                object = *const_cast<osg::Object**>(reinterpret_cast<const osg::Object**>(lua_touserdata(_lua,-1)));
            }

            lua_pop(_lua,1);

            return lse->setPropertyFromStack(object, propertyName);
        }
    }

    OSG_NOTICE<<"Warning: Lua setProperty() not matched"<<std::endl;
    return 0;
}

static int callClassMethod(lua_State* _lua)
{
    const LuaScriptEngine* lse = reinterpret_cast<const LuaScriptEngine*>(lua_topointer(_lua, lua_upvalueindex(1)));
    std::string methodName = lua_tostring(_lua, lua_upvalueindex(2));
    int n = lua_gettop(_lua);    /* number of arguments */

    if (n>=1 && lua_type(_lua, 1)==LUA_TTABLE)
    {
        osg::Object* object  = lse->getObjectFromTable<osg::Object>(1);
        const std::string compoundClassName = lse->getObjectCompoundClassName(1); // object->getCompoundClassName();
        // OSG_NOTICE<<"callClassMethod() on "<<object->className()<<" method name "<<methodName<<", stored compoundClassName "<<compoundClassName<<std::endl;

        // need to put within a c function
        osg::Parameters inputParameters, outputParameters;
        for(int i=2; i<=n; ++i)
        {
            // OSG_NOTICE<<" need to push parameter "<<lua_typename(_lua, lua_type(_lua, n))<<std::endl;
            inputParameters.insert(inputParameters.begin(), lse->popParameterObject());
        }

        if (lse->getPropertyInterface().run(object, compoundClassName, methodName, inputParameters, outputParameters))
        {
            for(osg::Parameters::iterator itr = outputParameters.begin();
                itr != outputParameters.end();
                ++itr)
            {
                // OSG_NOTICE<<" pushing return "<<(*itr)->className()<<std::endl;
                lse->pushParameter(itr->get());
            }
            return outputParameters.size();
        }
    }
    else
    {
        OSG_NOTICE<<"Warning: lua method called without passing object, use object::method() convention."<<std::endl;
    }

    return 0;
}

static int garabageCollectObject(lua_State* _lua)
{
    int n = lua_gettop(_lua);    /* number of arguments */
    if (n==1)
    {
        if (lua_type(_lua, 1)==LUA_TUSERDATA)
        {
            osg::Object* object = *const_cast<osg::Object**>(reinterpret_cast<const osg::Object**>(lua_touserdata(_lua, 1)));
            object->unref();
        }
    }

    return 0;
}

static int newObject(lua_State * _lua)
{
    const LuaScriptEngine* lse = reinterpret_cast<const LuaScriptEngine*>(lua_topointer(_lua, lua_upvalueindex(1)));

    int n = lua_gettop(_lua);    /* number of arguments */
    if (n==1)
    {
        if (lua_type(_lua, 1)==LUA_TSTRING)
        {
            std::string compoundName = lua_tostring(_lua, 1);

            lse->createAndPushObject(compoundName);
            return 1;
        }
    }
    return 0;
}

static int castObject(lua_State * _lua)
{
    const LuaScriptEngine* lse = reinterpret_cast<const LuaScriptEngine*>(lua_topointer(_lua, lua_upvalueindex(1)));

    int n = lua_gettop(_lua);    /* number of arguments */
    if (n==2)
    {
        if (lua_type(_lua, 1)==LUA_TSTRING && lua_type(_lua, 2)==LUA_TTABLE)
        {
            std::string new_compoundClassName = lua_tostring(_lua, 1);
            osg::Object* object  = lse->getObjectFromTable<osg::Object>(2);

            lse->pushAndCastObject(new_compoundClassName, object);

            return 1;
        }
    }
    return 0;
}

static int readObjectFile(lua_State * _lua)
{
    const LuaScriptEngine* lse = reinterpret_cast<const LuaScriptEngine*>(lua_topointer(_lua, lua_upvalueindex(1)));

    int n = lua_gettop(_lua);    /* number of arguments */
    if (n==1 && lua_type(_lua, 1)==LUA_TSTRING)
    {
        std::string filename = lua_tostring(_lua, 1);
        osg::ref_ptr<osg::Object> object = osgDB::readObjectFile(filename);
        if (object.valid())
        {
            lse->pushObject(object.get());
            return 1;
        }
    }
    return 0;
}

static int readImageFile(lua_State * _lua)
{
    const LuaScriptEngine* lse = reinterpret_cast<const LuaScriptEngine*>(lua_topointer(_lua, lua_upvalueindex(1)));

    int n = lua_gettop(_lua);    /* number of arguments */
    if (n==1 && lua_type(_lua, 1)==LUA_TSTRING)
    {
        std::string filename = lua_tostring(_lua, 1);
        osg::ref_ptr<osg::Image> image = osgDB::readImageFile(filename);
        if (image.valid())
        {
            lse->pushObject(image.get());
            return 1;
        }
    }
    return 0;
}

static int readNodeFile(lua_State * _lua)
{
    const LuaScriptEngine* lse = reinterpret_cast<const LuaScriptEngine*>(lua_topointer(_lua, lua_upvalueindex(1)));

    int n = lua_gettop(_lua);    /* number of arguments */
    if (n==1 && lua_type(_lua, 1)==LUA_TSTRING)
    {
        std::string filename = lua_tostring(_lua, 1);
        osg::ref_ptr<osg::Node> node = osgDB::readNodeFile(filename);
        if (node.valid())
        {
            lse->pushObject(node.get());
            return 1;
        }
    }
    return 0;
}

LuaScriptEngine::LuaScriptEngine():
    osg::ScriptEngine("lua"),
    _lua(0),
    _scriptCount(0)
{
    initialize();
}

LuaScriptEngine::LuaScriptEngine(const LuaScriptEngine& rhs, const osg::CopyOp&):
    osg::ScriptEngine("lua"),
    _lua(0),
    _scriptCount(0)
{
    initialize();
}

LuaScriptEngine::~LuaScriptEngine()
{
    lua_close(_lua);
}

std::string LuaScriptEngine::createUniquieScriptName()
{
    std::stringstream sstr;
    sstr<<"script_"<<_scriptCount;
    ++_scriptCount;

    return sstr.str();
}

void LuaScriptEngine::initialize()
{
    _lua = luaL_newstate();

    luaL_openlibs(_lua);

    // provide global new method for creating osg::Object's.
    {
        lua_pushlightuserdata(_lua, this);
        lua_pushcclosure(_lua, newObject, 1);
        lua_setglobal(_lua, "new");
    }

    // provide global new method for casting osg::Object's.
    {
        lua_pushlightuserdata(_lua, this);
        lua_pushcclosure(_lua, castObject, 1);
        lua_setglobal(_lua, "cast");
    }

    // provide global new method for reading Objects
    {
        lua_pushlightuserdata(_lua, this);
        lua_pushcclosure(_lua, readObjectFile, 1);
        lua_setglobal(_lua, "readObjectFile");
    }

    // provide global new method for reading Nodes
    {
        lua_pushlightuserdata(_lua, this);
        lua_pushcclosure(_lua, readNodeFile, 1);
        lua_setglobal(_lua, "readNodeFile");
    }

    // provide global new method for read Images
    {
        lua_pushlightuserdata(_lua, this);
        lua_pushcclosure(_lua, readImageFile, 1);
        lua_setglobal(_lua, "readImageFile");
    }

    // Set up the __newindex and __index methods for looking up implementations of Object properties
    {
        luaL_newmetatable(_lua, "LuaScriptEngine.Object");

        lua_pushstring(_lua, "__index");
        lua_pushlightuserdata(_lua, this);
        lua_pushcclosure(_lua, getProperty, 1);
        lua_settable(_lua, -3);

        lua_pushstring(_lua, "__newindex");
        lua_pushlightuserdata(_lua, this);
        lua_pushcclosure(_lua, setProperty, 1);
        lua_settable(_lua, -3);

        lua_pop(_lua,1);
    }

    // Set up the __gc methods for looking up implementations of Object pointer to do the unref when the associated Lua object is destroyed.
    {
        luaL_newmetatable(_lua, "LuaScriptEngine.UnrefObject");
        lua_pushstring(_lua, "__gc");
        lua_pushlightuserdata(_lua, this);
        lua_pushcclosure(_lua, garabageCollectObject, 1);
        lua_settable(_lua, -3);

        lua_pop(_lua,1);
    }
}

bool LuaScriptEngine::loadScript(osg::Script* script)
{
    if (_loadedScripts.count(script)!=0) return true;

    int loadResult = luaL_loadstring(_lua, script->getScript().c_str());
    if (loadResult==0)
    {
        std::string scriptID = createUniquieScriptName();

        lua_pushvalue(_lua, -1);
        lua_setglobal(_lua, scriptID.c_str());

        _loadedScripts[script] = scriptID;

        return true;
    }
    else
    {
        OSG_NOTICE << "LuaScriptEngine::luaL_loadstring(Script*) error: " << lua_tostring(_lua, -1) << std::endl;
        return false;
    }
}


bool LuaScriptEngine::run(osg::Script* script, const std::string& entryPoint, osg::Parameters& inputParameters, osg::Parameters& outputParameters)
{
    if (!script || !_lua) return false;


    if (_loadedScripts.count(script)==0)
    {
        if (!loadScript(script)) return false;

        if (!entryPoint.empty())
        {
            if (lua_pcall(_lua, 0, 0, 0)!=0)
            {
                OSG_NOTICE<< "error initialize script "<< lua_tostring(_lua, -1)<<std::endl;
                return false;
            }
        }
    }

    int topBeforeCall = lua_gettop(_lua);

    if (entryPoint.empty())
    {
        ScriptMap::iterator itr = _loadedScripts.find(script);
        if (itr == _loadedScripts.end()) return false;

        std::string scriptID = itr->second;

        lua_getglobal(_lua, scriptID.c_str());
    }
    else
    {
        lua_getglobal(_lua, entryPoint.c_str()); /* function to be called */
    }

    for(osg::Parameters::const_iterator itr = inputParameters.begin();
        itr != inputParameters.end();
        ++itr)
    {
        pushParameter(itr->get());
    }


    if (lua_pcall(_lua, inputParameters.size(), LUA_MULTRET,0)!=0)
    {
        OSG_NOTICE<<"Lua error : "<<lua_tostring(_lua, -1)<<std::endl;
        return false;
    }

    int topAfterCall = lua_gettop(_lua);
    int numReturns = topAfterCall-topBeforeCall;

    outputParameters.clear();

    for(int i=0; i<numReturns; ++i)
    {
        osg::ref_ptr<osg::Object> obj = popParameterObject();
        if (obj.valid()) outputParameters.push_back(obj);
    }

    return true;
}


class PushStackValueVisitor : public osg::ValueObject::GetValueVisitor
{
public:

    const LuaScriptEngine* _lsg;
    lua_State* _lua;

    PushStackValueVisitor(const LuaScriptEngine* lsg) : _lsg(lsg) { _lua = const_cast<LuaScriptEngine*>(lsg)->getLuaState(); }

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
    virtual void apply(const osg::Vec2f& value)         { _lsg->pushValue(value); }
    virtual void apply(const osg::Vec3f& value)         { _lsg->pushValue(value); }
    virtual void apply(const osg::Vec4f& value)         { _lsg->pushValue(value); }
    virtual void apply(const osg::Vec2d& value)         { _lsg->pushValue(value); }
    virtual void apply(const osg::Vec3d& value)         { _lsg->pushValue(value); }
    virtual void apply(const osg::Vec4d& value)         { _lsg->pushValue(value); }
    virtual void apply(const osg::Quat& value)          { _lsg->pushValue(value); }
    virtual void apply(const osg::Plane& value)         { _lsg->pushValue(value); }
    virtual void apply(const osg::Matrixf& value)       { _lsg->pushValue(value); }
    virtual void apply(const osg::Matrixd& value)       { _lsg->pushValue(value); }
};

#if LUA_VERSION_NUM<=501
    #define lua_rawlen lua_strlen
#endif

class GetStackValueVisitor : public osg::ValueObject::SetValueVisitor
{
public:

    const LuaScriptEngine* _lsg;
    lua_State* _lua;
    int _index;
    int _numberToPop;

    GetStackValueVisitor(const LuaScriptEngine* lsg, int index) : _lsg(lsg), _lua(0), _index(index), _numberToPop(0) { _lua = const_cast<LuaScriptEngine*>(lsg)->getLuaState(); }


    virtual void apply(bool& value)             { if (lua_isboolean(_lua, _index)) { value = (lua_toboolean(_lua, _index)!=0); _numberToPop = 1; } }
    virtual void apply(char& value)             { if (lua_isnumber(_lua, _index)) { value = lua_tonumber(_lua, _index)!=0; _numberToPop = 1; } }
    virtual void apply(unsigned char& value)    { if (lua_isnumber(_lua, _index)) { value = lua_tonumber(_lua, _index)!=0; _numberToPop = 1; } }
    virtual void apply(short& value)            { if (lua_isnumber(_lua, _index)) { value = lua_tonumber(_lua, _index)!=0; _numberToPop = 1; } }
    virtual void apply(unsigned short& value)   { if (lua_isnumber(_lua, _index)) { value = lua_tonumber(_lua, _index)!=0; _numberToPop = 1; } }
    virtual void apply(int& value)              { if (lua_isnumber(_lua, _index)) { value = lua_tonumber(_lua, _index)!=0; _numberToPop = 1; } }
    virtual void apply(unsigned int& value)     { if (lua_isnumber(_lua, _index)) { value = lua_tonumber(_lua, _index)!=0; _numberToPop = 1; } }
    virtual void apply(float& value)            { if (lua_isnumber(_lua, _index)) { value = lua_tonumber(_lua, _index)!=0; _numberToPop = 1; } }
    virtual void apply(double& value)           { if (lua_isnumber(_lua, _index)) { value = lua_tonumber(_lua, _index)!=0; _numberToPop = 1; } }
    virtual void apply(std::string& value)      { if (lua_isstring(_lua, _index)) { value = std::string(lua_tostring(_lua, _index), lua_rawlen(_lua, _index)); _numberToPop = 1; } }
    virtual void apply(osg::Vec2f& value)       { _lsg->getValue(value); _numberToPop = 2;}
    virtual void apply(osg::Vec3f& value)       { _lsg->getValue(value); _numberToPop = 2; }
    virtual void apply(osg::Vec4f& value)       { _lsg->getValue(value); _numberToPop = 4; }
    virtual void apply(osg::Vec2d& value)       { _lsg->getValue(value); _numberToPop = 2; }
    virtual void apply(osg::Vec3d& value)       { _lsg->getValue(value); _numberToPop = 3; }
    virtual void apply(osg::Vec4d& value)       { _lsg->getValue(value); _numberToPop = 4; }
    virtual void apply(osg::Quat& value)        { _lsg->getValue(value); _numberToPop = 4; }
    virtual void apply(osg::Plane& value)       { _lsg->getValue(value); _numberToPop = 4; }
    virtual void apply(osg::Matrixf& value)     { _lsg->getValue(value); }
    virtual void apply(osg::Matrixd& value)     { _lsg->getValue(value); }
    virtual void apply(osg::BoundingBoxf& value) { _lsg->getValue(value); }
    virtual void apply(osg::BoundingBoxd& value) { _lsg->getValue(value); }
    virtual void apply(osg::BoundingSpheref& value) { _lsg->getValue(value); }
    virtual void apply(osg::BoundingSphered& value) { _lsg->getValue(value); }
};

int LuaScriptEngine::pushPropertyToStack(osg::Object* object, const std::string& propertyName) const
{
    OSG_NOTICE<<"LuaScriptEngine::pushPropertyToStack("<<object<<", "<<propertyName<<")"<<std::endl;

    osgDB::BaseSerializer::Type type;
    if (!_pi.getPropertyType(object, propertyName, type))
    {
        if (_pi.hasMethod(object, propertyName))
        {
            lua_pushlightuserdata(_lua, const_cast<LuaScriptEngine*>(this));
            lua_pushstring(_lua, propertyName.c_str());
            lua_pushcclosure(_lua, callClassMethod, 2);

            return 1;
        }

        osg::Object* uo = osg::getUserObject(object, propertyName);
        LuaCallbackObject* lco = dynamic_cast<LuaCallbackObject*>(uo);
        if (lco)
        {
            lua_rawgeti(_lua, LUA_REGISTRYINDEX, lco->getRef());
            return 1;
        }
        else if (uo)
        {
            pushObject(uo);
            return 1;
        }

        OSG_NOTICE<<"LuaScriptEngine::pushPropertyToStack("<<object<<", "<<propertyName<<") no property found."<<std::endl;
        return 0;
    }

    switch(type)
    {
        case(osgDB::BaseSerializer::RW_BOOL):
        {
            bool value;
            if (_pi.getProperty(object, propertyName, value))
            {
                lua_pushboolean(_lua, value ? 1 : 0);
                return 1;
            }
            break;
        }
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
        {
            int value;
            if (_pi.getProperty(object, propertyName, value))
            {
                osgDB::BaseSerializer* serializer = _pi.getSerializer(object, propertyName, type);
                osgDB::IntLookup* lookup = serializer ? serializer->getIntLookup() : 0;
                if (lookup)
                {
                    std::string enumString = lookup->getString(value);
                    lua_pushstring(_lua, enumString.c_str());
                }
                else
                {
                    lua_pushinteger(_lua, value);
                }
                return 1;
            }
            break;
        }
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
                pushValue(value);
                return 1;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_VEC3F):
        {
            osg::Vec3f value;
            if (_pi.getProperty(object, propertyName, value))
            {
                pushValue(value);
                return 1;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_VEC4F):
        {
            osg::Vec4f value;
            if (_pi.getProperty(object, propertyName, value))
            {
                pushValue(value);
                return 1;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_VEC2D):
        {
            osg::Vec2d value;
            if (_pi.getProperty(object, propertyName, value))
            {
                pushValue(value);
                return 1;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_VEC3D):
        {
            osg::Vec3d value;
            if (_pi.getProperty(object, propertyName, value))
            {
                pushValue(value);
                return 1;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_VEC4D):
        {
            osg::Vec4d value;
            if (_pi.getProperty(object, propertyName, value))
            {
                pushValue(value);
                return 1;
            }
            break;
        }
#ifdef OSG_USE_FLOAT_MATRIX
        case(osgDB::BaseSerializer::RW_MATRIX):
#endif
        case(osgDB::BaseSerializer::RW_MATRIXF):
        {
            osg::Matrixf value;
            if (_pi.getProperty(object, propertyName, value))
            {
                pushValue(value);
                return 1;
            }
            break;
        }
#ifndef OSG_USE_FLOAT_MATRIX
        case(osgDB::BaseSerializer::RW_MATRIX):
#endif
        case(osgDB::BaseSerializer::RW_MATRIXD):
        {
            osg::Matrixd value;
            if (_pi.getProperty(object, propertyName, value))
            {
                pushValue(value);
                return 1;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_BOUNDINGBOXF):
        {
            osg::BoundingBoxf value;
            if (_pi.getProperty(object, propertyName, value))
            {
                pushValue(value);
                return 1;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_BOUNDINGBOXD):
        {
            osg::BoundingBoxd value;
            if (_pi.getProperty(object, propertyName, value))
            {
                pushValue(value);
                return 1;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_BOUNDINGSPHEREF):
        {
            osg::BoundingSpheref value;
            if (_pi.getProperty(object, propertyName, value))
            {
                pushValue(value);
                return 1;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_BOUNDINGSPHERED):
        {
            osg::BoundingSphered value;
            if (_pi.getProperty(object, propertyName, value))
            {
                pushValue(value);
                return 1;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_LIST):
        {
            OSG_NOTICE<<"Need to implement RW_LIST support"<<std::endl;
            break;
        }
        case(osgDB::BaseSerializer::RW_IMAGE):
        case(osgDB::BaseSerializer::RW_OBJECT):
        {
            osg::Object* value = 0;
            if (_pi.getProperty(object, propertyName, value))
            {
                pushObject(value);
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
        if (lua_type(_lua,-1)==LUA_TFUNCTION)
        {
            int ref = luaL_ref(_lua, LUA_REGISTRYINDEX);

            osg::ref_ptr<LuaCallbackObject> lco = new LuaCallbackObject(propertyName, this, ref);
            object->getOrCreateUserDataContainer()->addUserObject(lco.get());
            return 0;
        }

        type = LuaScriptEngine::getType();
    }

    switch(type)
    {
        case(osgDB::BaseSerializer::RW_BOOL):
        {
            if (lua_isboolean(_lua, -1))
            {
                _pi.setProperty(object, propertyName, static_cast<bool>(lua_toboolean(_lua, -1)));
                return 0;
            }
            else if (lua_isnumber(_lua, -1))
            {
                _pi.setProperty(object, propertyName, static_cast<bool>(lua_tonumber(_lua, -1)!=0));
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
        {
            if (lua_isnumber(_lua, -1))
            {
                _pi.setProperty(object, propertyName, static_cast<int>(lua_tonumber(_lua, -1)));
                return 0;
            }
            else if (lua_isstring(_lua, -1))
            {
                const char* enumString = lua_tostring(_lua, -1);
                osgDB::BaseSerializer* serializer = _pi.getSerializer(object, propertyName, type);
                osgDB::IntLookup* lookup = serializer ? serializer->getIntLookup() : 0;
                if (lookup)
                {
                    int value = lookup->getValue(enumString);
                    _pi.setProperty(object, propertyName, value);
                }
                return 0;
            }
            break;
        }
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
        case(osgDB::BaseSerializer::RW_VEC2D):
        {
            osg::Vec2d value;
            if (getValue(value))
            {
                _pi.setProperty(object, propertyName, value);
                return 0;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_VEC3D):
        {
            osg::Vec3d value;
            if (getValue(value))
            {
                _pi.setProperty(object, propertyName, value);
                return 0;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_VEC4D):
        {
            osg::Vec4d value;
            if (getValue(value))
            {
                _pi.setProperty(object, propertyName, value);
                return 0;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_QUAT):
        {
            osg::Quat value;
            if (getValue(value))
            {
                _pi.setProperty(object, propertyName, value);
                return 0;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_PLANE):
        {
            osg::Plane value;
            if (getValue(value))
            {
                _pi.setProperty(object, propertyName, value);
                return 0;
            }
            break;
        }
#ifdef OSG_USE_FLOAT_MATRIX
        case(osgDB::BaseSerializer::RW_MATRIX):
#endif
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
#ifndef OSG_USE_FLOAT_MATRIX
        case(osgDB::BaseSerializer::RW_MATRIX):
#endif
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
        case(osgDB::BaseSerializer::RW_BOUNDINGBOXF):
        {
            osg::BoundingBoxf value;
            if (getValue(value))
            {
                _pi.setProperty(object, propertyName, value);
                return 0;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_BOUNDINGBOXD):
        {
            osg::BoundingBoxd value;
            if (getValue(value))
            {
                _pi.setProperty(object, propertyName, value);
                return 0;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_BOUNDINGSPHEREF):
        {
            osg::BoundingSpheref value;
            if (getValue(value))
            {
                _pi.setProperty(object, propertyName, value);
                return 0;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_BOUNDINGSPHERED):
        {
            osg::BoundingSphered value;
            if (getValue(value))
            {
                _pi.setProperty(object, propertyName, value);
                return 0;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_LIST):
        {
            OSG_NOTICE<<"Need to implement RW_LIST support"<<std::endl;
            break;
        }
        case(osgDB::BaseSerializer::RW_IMAGE):
        case(osgDB::BaseSerializer::RW_OBJECT):
        {
            if (lua_istable(_lua, -1))
            {
                osg::Object* value = 0;
                lua_pushstring(_lua, "object_ptr");
                lua_rawget(_lua, -2);
                if (lua_type(_lua, -1)==LUA_TUSERDATA) value = *const_cast<osg::Object**>(reinterpret_cast<const osg::Object**>(lua_touserdata(_lua,-1)));
                lua_pop(_lua, 1);

                if (value)
                {
                    _pi.setProperty(object, propertyName, value);
                    return 0;
                }
                else
                {
                    OSG_NOTICE<<"Error: lua type '"<<lua_typename(_lua,lua_type(_lua, -1))<<"' cannot be assigned to "<<object->className()<<"::"<<propertyName<<std::endl;
                }
            }
            else if (lua_isnil(_lua, -1))
            {
                OSG_NOTICE<<"Assigning property object (nil) to to object "<<object->className()<<"::"<<propertyName<<std::endl;
                osg::Object* value = 0;
                _pi.setProperty(object, propertyName, value);
                return 0;
            }
            else
            {
                OSG_NOTICE<<"Error: lua type '"<<lua_typename(_lua,lua_type(_lua, -1))<<"' cannot be assigned to "<<object->className()<<"::"<<propertyName<<std::endl;
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

bool LuaScriptEngine::getfields(const char* f1, const char* f2, const char* f3, const char* f4, const char* f5, const char* f6, int type) const
{
    lua_getfield(_lua, -1, f1);
    if (lua_type(_lua, -1)!=type) { lua_pop(_lua, 1); return false; }

    lua_getfield(_lua, -2, f2);
    if (lua_type(_lua, -1)!=type) { lua_pop(_lua, 2); return false; }

    lua_getfield(_lua, -3, f3);
    if (lua_type(_lua, -1)!=type) { lua_pop(_lua, 3); return false; }

    lua_getfield(_lua, -4, f4);
    if (lua_type(_lua, -1)!=type) { lua_pop(_lua, 4); return false; }

    lua_getfield(_lua, -5, f5);
    if (lua_type(_lua, -1)!=type) { lua_pop(_lua, 5); return false; }

    lua_getfield(_lua, -6, f6);
    if (lua_type(_lua, -1)!=type) { lua_pop(_lua, 6); return false; }

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


osgDB::BaseSerializer::Type LuaScriptEngine::getType() const
{
    switch(lua_type(_lua,-1))
    {
        case(LUA_TNIL): return osgDB::BaseSerializer::RW_UNDEFINED;
        case(LUA_TNUMBER): return osgDB::BaseSerializer::RW_DOUBLE;
        case(LUA_TBOOLEAN): return osgDB::BaseSerializer::RW_BOOL;
        case(LUA_TSTRING): return osgDB::BaseSerializer::RW_STRING;
        case(LUA_TTABLE):
        {
            lua_pushstring(_lua, "object_ptr");
            lua_rawget(_lua, -2);
            bool isObject = (lua_type(_lua, -1)==LUA_TUSERDATA);
            lua_pop(_lua, 1);

            if (isObject)
            {
                return osgDB::BaseSerializer::RW_OBJECT;
            }


            int n = lua_gettop(_lua);    /* number of arguments */
            lua_pushnil(_lua);

            int numStringKeys = 0;
            int numNumberKeys = 0;
            int numNumberFields = 0;

            while (lua_next(_lua, n) != 0)
            {
                if (lua_type(_lua, -2)==LUA_TSTRING) ++numStringKeys;
                else if (lua_type(_lua, -2)==LUA_TNUMBER) ++numNumberKeys;

                if (lua_type(_lua, -1)==LUA_TNUMBER) ++numNumberFields;

                lua_pop(_lua, 1); // remove value, leave key for next iteration
            }

            if ((numStringKeys==2 || numNumberKeys==2) && (numNumberFields==2))
            {
                OSG_NOTICE<<"Could be Vec2d"<<std::endl;
                return osgDB::BaseSerializer::RW_VEC2D;
            }
            else if ((numStringKeys==3 || numNumberKeys==3) && (numNumberFields==3))
            {
                OSG_NOTICE<<"Could be Vec3d"<<std::endl;
                return osgDB::BaseSerializer::RW_VEC3D;
            }
            else if ((numStringKeys==4 || numNumberKeys==4) && (numNumberFields==4))
            {
                OSG_NOTICE<<"Could be Vec4d"<<std::endl;
                return osgDB::BaseSerializer::RW_VEC4D;
            }
            else if ((numNumberKeys==16) && (numNumberFields==16))
            {
                OSG_NOTICE<<"Could be Matrixd"<<std::endl;
                return osgDB::BaseSerializer::RW_MATRIXD;
            }
            else if ((numNumberKeys==6) && (numNumberFields==6))
            {
                OSG_NOTICE<<"Could be BoundingBoxd"<<std::endl;
                return osgDB::BaseSerializer::RW_BOUNDINGBOXD;
            }
            // not supported
            OSG_NOTICE<<"Warning: LuaScriptEngine::getType() Lua table configuration not supported."<<std::endl;
            break;
        }
        default:
            OSG_NOTICE<<"Warning: LuaScriptEngine::getType() Lua type "<<lua_typename(_lua, lua_type(_lua, -1))<<" not supported."<<std::endl;
            break;

    }
    return osgDB::BaseSerializer::RW_UNDEFINED;
}

bool LuaScriptEngine::getvec2() const
{
    if (lua_istable(_lua, -1))
    {
        if (getfields("x", "y", LUA_TNUMBER) ||
            getfields("s", "t", LUA_TNUMBER) ||
            getelements(2, LUA_TNUMBER))
        {
            return true;
        }
    }
    return false;
}

bool LuaScriptEngine::getvec3() const
{
    if (lua_istable(_lua, -1))
    {
        if (getfields("x", "y", "z", LUA_TNUMBER) ||
            getfields("r", "g", "b", LUA_TNUMBER) ||
            getfields("red", "green", "blue", LUA_TNUMBER) ||
            getfields("s", "t", "r", LUA_TNUMBER) ||
            getelements(3, LUA_TNUMBER))
        {
            return true;
        }
    }
    return false;
}
bool LuaScriptEngine::getvec4() const
{
    if (lua_istable(_lua, -1))
    {
        if (getfields("x", "y", "z", LUA_TNUMBER) ||
            getfields("r", "g", "b", "a", LUA_TNUMBER) ||
            getfields("red", "green", "blue", "alpha", LUA_TNUMBER) ||
            getfields("s", "t", "r", "q", LUA_TNUMBER) ||
            getelements(4, LUA_TNUMBER))
        {
            return true;
        }
    }
    return false;
}

bool LuaScriptEngine::getmatrix() const
{
    if (lua_istable(_lua, -1))
    {
        if (getelements(16,LUA_TNUMBER))
        {
            return true;
        }
    }
    return false;
}

bool LuaScriptEngine::getboundingbox() const
{
    if (lua_istable(_lua, -1))
    {
        if (getfields("xMin", "yMin", "zMin", "xMax", "yMax", "zMax", LUA_TNUMBER) ||
            getelements(6, LUA_TNUMBER))
        {
            return true;
        }
    }
    return false;
}

bool LuaScriptEngine::getboundingsphere() const
{
    if (lua_istable(_lua, -1))
    {
        if (getfields("x", "y", "z", "radius", LUA_TNUMBER) ||
            getelements(4, LUA_TNUMBER))
        {
            return true;
        }
    }
    return false;
}

bool LuaScriptEngine::getValue(osg::Vec2f& value) const
{
    if (!getvec2()) return false;

    value.set(lua_tonumber(_lua, -2), lua_tonumber(_lua, -1));
    lua_pop(_lua, 2);

    return true;
}

bool LuaScriptEngine::getValue(osg::Vec3f& value) const
{
    if (!getvec3()) return false;
    value.set(lua_tonumber(_lua, -3), lua_tonumber(_lua, -2), lua_tonumber(_lua, -1));
    lua_pop(_lua, 3);
    return true;
}

bool LuaScriptEngine::getValue(osg::Vec4f& value) const
{
    if (!getvec4()) return false;
    value.set(lua_tonumber(_lua, -4), lua_tonumber(_lua, -3), lua_tonumber(_lua, -2), lua_tonumber(_lua, -1));
    lua_pop(_lua, 4);
    return true;
}

bool LuaScriptEngine::getValue(osg::Matrixf& value) const
{
    if (!getmatrix()) return false;

    for(int r=0; r<4; ++r)
    {
        for(int c=0; c<4; ++c)
        {
            value(r,c) = lua_tonumber(_lua, -16+(r*4+c));
        }
    }
    return true;
}

bool LuaScriptEngine::getValue(osg::Vec2d& value) const
{
    if (!getvec2()) return false;

    value.set(lua_tonumber(_lua, -2), lua_tonumber(_lua, -1));
    lua_pop(_lua, 2);

    return true;
}

bool LuaScriptEngine::getValue(osg::Vec3d& value) const
{
    if (!getvec3()) return false;
    value.set(lua_tonumber(_lua, -3), lua_tonumber(_lua, -2), lua_tonumber(_lua, -1));
    lua_pop(_lua, 3);
    return true;
}

bool LuaScriptEngine::getValue(osg::Vec4d& value) const
{
    if (!getvec4()) return false;
    value.set(lua_tonumber(_lua, -4), lua_tonumber(_lua, -3), lua_tonumber(_lua, -2), lua_tonumber(_lua, -1));
    lua_pop(_lua, 4);
    return true;
}

bool LuaScriptEngine::getValue(osg::Quat& value) const
{
    if (!getvec4()) return false;
    value.set(lua_tonumber(_lua, -4), lua_tonumber(_lua, -3), lua_tonumber(_lua, -2), lua_tonumber(_lua, -1));
    lua_pop(_lua, 4);
    return true;
}

bool LuaScriptEngine::getValue(osg::Plane& value) const
{
    if (!getvec4()) return false;
    value.set(lua_tonumber(_lua, -4), lua_tonumber(_lua, -3), lua_tonumber(_lua, -2), lua_tonumber(_lua, -1));
    lua_pop(_lua, 4);
    return true;
}

bool LuaScriptEngine::getValue(osg::Matrixd& value) const
{
    if (!getmatrix()) return false;

    for(int r=0; r<4; ++r)
    {
        for(int c=0; c<4; ++c)
        {
            value(r,c) = lua_tonumber(_lua, -16+(r*4+c));
        }
    }
    return true;
}

bool LuaScriptEngine::getValue(osg::BoundingBoxf& value) const
{
    if (!getboundingbox()) return false;
    value.set(lua_tonumber(_lua, -6), lua_tonumber(_lua, -5), lua_tonumber(_lua, -4), lua_tonumber(_lua, -3), lua_tonumber(_lua, -2), lua_tonumber(_lua, -1));
    lua_pop(_lua, 6);
    return true;
}

bool LuaScriptEngine::getValue(osg::BoundingBoxd& value) const
{
    if (!getboundingbox()) return false;
    value.set(lua_tonumber(_lua, -6), lua_tonumber(_lua, -5), lua_tonumber(_lua, -4), lua_tonumber(_lua, -3), lua_tonumber(_lua, -2), lua_tonumber(_lua, -1));
    lua_pop(_lua, 6);
    return true;
}

bool LuaScriptEngine::getValue(osg::BoundingSpheref& value) const
{
    if (!getboundingsphere()) return false;
    value.set(osg::Vec3f(lua_tonumber(_lua, -4), lua_tonumber(_lua, -3), lua_tonumber(_lua, -2)), lua_tonumber(_lua, -1));
    lua_pop(_lua, 4);
    return true;
}

bool LuaScriptEngine::getValue(osg::BoundingSphered& value) const
{
    if (!getboundingsphere()) return false;
    value.set(osg::Vec3d(lua_tonumber(_lua, -4), lua_tonumber(_lua, -3), lua_tonumber(_lua, -2)), lua_tonumber(_lua, -1));
    lua_pop(_lua, 4);
    return true;
}

void LuaScriptEngine::pushValue(const osg::Vec2f& value) const
{
    lua_newtable(_lua);
    lua_pushstring(_lua, "x"); lua_pushnumber(_lua, value.x()); lua_settable(_lua, -3);
    lua_pushstring(_lua, "y"); lua_pushnumber(_lua, value.y()); lua_settable(_lua, -3);
}

void LuaScriptEngine::pushValue(const osg::Vec3f& value) const
{
    lua_newtable(_lua);
    lua_pushstring(_lua, "x"); lua_pushnumber(_lua, value.x()); lua_settable(_lua, -3);
    lua_pushstring(_lua, "y"); lua_pushnumber(_lua, value.y()); lua_settable(_lua, -3);
    lua_pushstring(_lua, "z"); lua_pushnumber(_lua, value.z()); lua_settable(_lua, -3);
}

void LuaScriptEngine::pushValue(const osg::Vec4f& value) const
{
    lua_newtable(_lua);
    lua_pushstring(_lua, "x"); lua_pushnumber(_lua, value.x()); lua_settable(_lua, -3);
    lua_pushstring(_lua, "y"); lua_pushnumber(_lua, value.y()); lua_settable(_lua, -3);
    lua_pushstring(_lua, "z"); lua_pushnumber(_lua, value.z()); lua_settable(_lua, -3);
    lua_pushstring(_lua, "w"); lua_pushnumber(_lua, value.w()); lua_settable(_lua, -3);
}

void LuaScriptEngine::pushValue(const osg::Matrixf& value) const
{
    lua_newtable(_lua);

    for(unsigned int r=0; r<4; ++r)
    {
        for(unsigned int c=0; c<4; ++c)
        {
            lua_pushnumber(_lua, r*4+c); lua_pushinteger(_lua, value(r,c)); lua_settable(_lua, -3);
        }
    }
}

void LuaScriptEngine::pushValue(const osg::Vec2d& value) const
{
    lua_newtable(_lua);
    lua_pushstring(_lua, "x"); lua_pushnumber(_lua, value.x()); lua_settable(_lua, -3);
    lua_pushstring(_lua, "y"); lua_pushnumber(_lua, value.y()); lua_settable(_lua, -3);
}

void LuaScriptEngine::pushValue(const osg::Vec3d& value) const
{
    lua_newtable(_lua);
    lua_pushstring(_lua, "x"); lua_pushnumber(_lua, value.x()); lua_settable(_lua, -3);
    lua_pushstring(_lua, "y"); lua_pushnumber(_lua, value.y()); lua_settable(_lua, -3);
    lua_pushstring(_lua, "z"); lua_pushnumber(_lua, value.z()); lua_settable(_lua, -3);
}

void LuaScriptEngine::pushValue(const osg::Vec4d& value) const
{
    lua_newtable(_lua);
    lua_pushstring(_lua, "x"); lua_pushnumber(_lua, value.x()); lua_settable(_lua, -3);
    lua_pushstring(_lua, "y"); lua_pushnumber(_lua, value.y()); lua_settable(_lua, -3);
    lua_pushstring(_lua, "z"); lua_pushnumber(_lua, value.z()); lua_settable(_lua, -3);
    lua_pushstring(_lua, "w"); lua_pushnumber(_lua, value.w()); lua_settable(_lua, -3);
}

void LuaScriptEngine::pushValue(const osg::Quat& value) const
{
    lua_newtable(_lua);
    lua_pushstring(_lua, "x"); lua_pushnumber(_lua, value.x()); lua_settable(_lua, -3);
    lua_pushstring(_lua, "y"); lua_pushnumber(_lua, value.y()); lua_settable(_lua, -3);
    lua_pushstring(_lua, "z"); lua_pushnumber(_lua, value.z()); lua_settable(_lua, -3);
    lua_pushstring(_lua, "w"); lua_pushnumber(_lua, value.w()); lua_settable(_lua, -3);
}

void LuaScriptEngine::pushValue(const osg::Plane& value) const
{
    lua_newtable(_lua);
    lua_pushstring(_lua, "x"); lua_pushnumber(_lua, value.asVec4().x()); lua_settable(_lua, -3);
    lua_pushstring(_lua, "y"); lua_pushnumber(_lua, value.asVec4().y()); lua_settable(_lua, -3);
    lua_pushstring(_lua, "z"); lua_pushnumber(_lua, value.asVec4().z()); lua_settable(_lua, -3);
    lua_pushstring(_lua, "w"); lua_pushnumber(_lua, value.asVec4().w()); lua_settable(_lua, -3);
}

void LuaScriptEngine::pushValue(const osg::Matrixd& value) const
{
    lua_newtable(_lua);

    for(unsigned int r=0; r<4; ++r)
    {
        for(unsigned int c=0; c<4; ++c)
        {
            lua_pushnumber(_lua, r*4+c); lua_pushinteger(_lua, value(r,c)); lua_settable(_lua, -3);
        }
    }
}

void LuaScriptEngine::pushValue(const osg::BoundingBoxf& value) const
{
    lua_newtable(_lua);
    lua_pushstring(_lua, "xMin"); lua_pushnumber(_lua, value.xMin()); lua_settable(_lua, -3);
    lua_pushstring(_lua, "yMin"); lua_pushnumber(_lua, value.yMin()); lua_settable(_lua, -3);
    lua_pushstring(_lua, "zMin"); lua_pushnumber(_lua, value.zMin()); lua_settable(_lua, -3);
    lua_pushstring(_lua, "xMax"); lua_pushnumber(_lua, value.xMax()); lua_settable(_lua, -3);
    lua_pushstring(_lua, "yMax"); lua_pushnumber(_lua, value.yMax()); lua_settable(_lua, -3);
    lua_pushstring(_lua, "zMax"); lua_pushnumber(_lua, value.zMax()); lua_settable(_lua, -3);
}

void LuaScriptEngine::pushValue(const osg::BoundingBoxd& value) const
{
    lua_newtable(_lua);
    lua_pushstring(_lua, "xMin"); lua_pushnumber(_lua, value.xMin()); lua_settable(_lua, -3);
    lua_pushstring(_lua, "yMin"); lua_pushnumber(_lua, value.yMin()); lua_settable(_lua, -3);
    lua_pushstring(_lua, "zMin"); lua_pushnumber(_lua, value.zMin()); lua_settable(_lua, -3);
    lua_pushstring(_lua, "xMax"); lua_pushnumber(_lua, value.xMax()); lua_settable(_lua, -3);
    lua_pushstring(_lua, "yMax"); lua_pushnumber(_lua, value.yMax()); lua_settable(_lua, -3);
    lua_pushstring(_lua, "zMax"); lua_pushnumber(_lua, value.zMax()); lua_settable(_lua, -3);
}

void LuaScriptEngine::pushValue(const osg::BoundingSpheref& value) const
{
    lua_newtable(_lua);
    lua_pushstring(_lua, "x"); lua_pushnumber(_lua, value.center().x()); lua_settable(_lua, -3);
    lua_pushstring(_lua, "y"); lua_pushnumber(_lua, value.center().y()); lua_settable(_lua, -3);
    lua_pushstring(_lua, "z"); lua_pushnumber(_lua, value.center().z()); lua_settable(_lua, -3);
    lua_pushstring(_lua, "radius"); lua_pushnumber(_lua, value.radius()); lua_settable(_lua, -3);
}

void LuaScriptEngine::pushValue(const osg::BoundingSphered& value) const
{
    lua_newtable(_lua);
    lua_pushstring(_lua, "x"); lua_pushnumber(_lua, value.center().x()); lua_settable(_lua, -3);
    lua_pushstring(_lua, "y"); lua_pushnumber(_lua, value.center().y()); lua_settable(_lua, -3);
    lua_pushstring(_lua, "z"); lua_pushnumber(_lua, value.center().z()); lua_settable(_lua, -3);
    lua_pushstring(_lua, "radius"); lua_pushnumber(_lua, value.radius()); lua_settable(_lua, -3);
}

bool LuaScriptEngine::pushParameter(osg::Object* object) const
{
    osg::ValueObject* vo = dynamic_cast<osg::ValueObject*>(object);
    if (vo)
    {
        PushStackValueVisitor pvv(this);
        vo->get(pvv);
    }
    else
    {
        pushObject( object);
    }

    return false;
}

bool LuaScriptEngine::popParameter(osg::Object* object) const
{
    osg::ValueObject* vo = dynamic_cast<osg::ValueObject*>(object);
    if (vo)
    {
        GetStackValueVisitor pvv(this, -1);
        vo->set(pvv);
        lua_pop(_lua, pvv._numberToPop);
    }
    else
    {
        lua_pop(_lua, 1);
    }

    return false;
}

osg::Object* LuaScriptEngine::popParameterObject() const
{
    osg::ref_ptr<osg::Object> object = 0;

    osgDB::BaseSerializer::Type type = getType();
    switch(type)
    {
        case(osgDB::BaseSerializer::RW_BOOL):
        {
            if (lua_isboolean(_lua, -1)) object = new osg::BoolValueObject("", lua_toboolean(_lua, -1)!=0);
            break;
        }
        case(osgDB::BaseSerializer::RW_STRING):
        {
            if (lua_isstring(_lua, -1)) object = new osg::StringValueObject("", lua_tostring(_lua, -1));
            break;
        }
        case(osgDB::BaseSerializer::RW_ENUM):
            if (lua_isstring(_lua, -1))
            {
                object = new osg::StringValueObject("", lua_tostring(_lua, -1));
            }
            else if (lua_isnumber(_lua, -1))
            {
                object = new osg::IntValueObject("", static_cast<int>(lua_tonumber(_lua, -1)));
            }
            break;
        case(osgDB::BaseSerializer::RW_INT):
        {
            if (lua_isnumber(_lua, -1)) object = new osg::IntValueObject("", static_cast<int>(lua_tonumber(_lua, -1)));
            break;
        }
        case(osgDB::BaseSerializer::RW_UINT):
        {
            if (lua_isnumber(_lua, -1)) object = new osg::UIntValueObject("", static_cast<unsigned int>(lua_tonumber(_lua, -1)));
            break;
        }
        case(osgDB::BaseSerializer::RW_FLOAT):
        {
            if (lua_isnumber(_lua, -1)) object = new osg::FloatValueObject("", static_cast<float>(lua_tonumber(_lua, -1)));
            break;
        }
        case(osgDB::BaseSerializer::RW_DOUBLE):
        {
            if (lua_isnumber(_lua, -1)) object = new osg::DoubleValueObject("", static_cast<double>(lua_tonumber(_lua, -1)));
            break;
        }
        case(osgDB::BaseSerializer::RW_VEC2F):
        {
            osg::Vec2f value;
            if (getValue(value)) object = new osg::Vec2fValueObject("", value);
            break;
        }
        case(osgDB::BaseSerializer::RW_VEC3F):
        {
            osg::Vec3f value;
            if (getValue(value)) object = new osg::Vec3fValueObject("", value);
            break;
        }
        case(osgDB::BaseSerializer::RW_VEC4F):
        {
            osg::Vec4f value;
            if (getValue(value)) object = new osg::Vec4fValueObject("", value);
            break;
        }
#ifdef OSG_USE_FLOAT_MATRIX
        case(osgDB::BaseSerializer::RW_MATRIX):
#endif
        case(osgDB::BaseSerializer::RW_MATRIXF):
        {
            osg::Matrixf value;
            if (getValue(value)) object = new osg::MatrixfValueObject("", value);
            break;
        }
        case(osgDB::BaseSerializer::RW_VEC2D):
        {
            osg::Vec2d value;
            if (getValue(value)) object = new osg::Vec2dValueObject("", value);
            break;
        }
        case(osgDB::BaseSerializer::RW_VEC3D):
        {
            osg::Vec3d value;
            if (getValue(value)) object = new osg::Vec3dValueObject("", value);
            break;
        }
        case(osgDB::BaseSerializer::RW_VEC4D):
        {
            osg::Vec4d value;
            if (getValue(value)) object = new osg::Vec4dValueObject("", value);
            break;
        }
        case(osgDB::BaseSerializer::RW_QUAT):
        {
            osg::Quat value;
            if (getValue(value)) object = new osg::QuatValueObject("", value);
            break;
        }
        case(osgDB::BaseSerializer::RW_PLANE):
        {
            osg::Plane value;
            if (getValue(value)) object = new osg::PlaneValueObject("", value);
            break;
        }
#ifndef OSG_USE_FLOAT_MATRIX
        case(osgDB::BaseSerializer::RW_MATRIX):
#endif
        case(osgDB::BaseSerializer::RW_MATRIXD):
        {
            osg::Matrixd value;
            if (getValue(value)) object = new osg::MatrixdValueObject("", value);
            break;
        }
        case(osgDB::BaseSerializer::RW_BOUNDINGBOXF):
        {
            osg::BoundingBoxf value;
            if (getValue(value)) object = new osg::BoundingBoxfValueObject("", value);
            break;
        }
        case(osgDB::BaseSerializer::RW_BOUNDINGBOXD):
        {
            osg::BoundingBoxd value;
            if (getValue(value)) object = new osg::BoundingBoxdValueObject("", value);
            break;
        }
        case(osgDB::BaseSerializer::RW_BOUNDINGSPHEREF):
        {
            osg::BoundingSpheref value;
            if (getValue(value)) object = new osg::BoundingSpherefValueObject("", value);
            break;
        }
        case(osgDB::BaseSerializer::RW_BOUNDINGSPHERED):
        {
            osg::BoundingSphered value;
            if (getValue(value)) object = new osg::BoundingSpheredValueObject("", value);
            break;
        }
        case(osgDB::BaseSerializer::RW_LIST):
        {
            OSG_NOTICE<<"Need to implement RW_LIST support"<<std::endl;
            break;
        }
        case(osgDB::BaseSerializer::RW_IMAGE):
        case(osgDB::BaseSerializer::RW_OBJECT):
        {
            lua_pushstring(_lua, "object_ptr");
            lua_rawget(_lua, -2);
            if (lua_type(_lua, -1)==LUA_TUSERDATA)
            {
                object = *const_cast<osg::Object**>(reinterpret_cast<const osg::Object**>(lua_touserdata(_lua,-1)));
            }
            lua_pop(_lua, 1);
        }
        default:
            break;
    }

    lua_pop(_lua, 1);

    return object.release();

#if 0
    osg::ValueObject* vo = dynamic_cast<osg::ValueObject*>(object);
    if (vo)
    {
        GetStackValueVisitor pvv(this, -1);
        vo->set(pvv);
        lua_pop(_lua, pvv._numberToPop);
    }
    else
    {
        lua_pop(_lua, 1);
    }
#endif
    return object.release();
}

void LuaScriptEngine::createAndPushObject(const std::string& compoundName) const
{
    osg::ref_ptr<osg::Object> object = _pi.createObject(compoundName);
    if (!object) OSG_NOTICE<<"Failed to create object "<<compoundName<<std::endl;

    pushObject(object.get());

    object.release();
}

void LuaScriptEngine::pushObject(osg::Object* object) const
{
    if (object)
    {
        lua_newtable(_lua);

        // set up objbect_ptr to handle ref/unref of the object
        {
            lua_pushstring(_lua, "object_ptr");

            // create user data for pointer
            void* userdata = lua_newuserdata( _lua, sizeof(osg::Object*));
            (*reinterpret_cast<osg::Object**>(userdata)) = object;

            luaL_getmetatable( _lua, "LuaScriptEngine.UnrefObject");
            lua_setmetatable( _lua, -2 );

            lua_settable(_lua, -3);

            // increment the reference count as the lua now will unreference it once it's finished with the userdata for the pointer
            object->ref();
        }

        lua_pushstring(_lua, "libraryName"); lua_pushstring(_lua, object->libraryName()); lua_settable(_lua, -3);
        lua_pushstring(_lua, "className"); lua_pushstring(_lua, object->className()); lua_settable(_lua, -3);
        lua_pushstring(_lua, "compoundClassName"); lua_pushstring(_lua, object->getCompoundClassName().c_str()); lua_settable(_lua, -3);

        luaL_getmetatable(_lua, "LuaScriptEngine.Object");
        lua_setmetatable(_lua, -2);
    }
    else
    {
        lua_pushnil(_lua);
    }
}

void LuaScriptEngine::pushAndCastObject(const std::string& compoundClassName, osg::Object* object) const
{
    if (object && _pi.isObjectOfType(object, compoundClassName))
    {
        lua_newtable(_lua);

        // set up objbect_ptr to handle ref/unref of the object
        {
            lua_pushstring(_lua, "object_ptr");

            // create user data for pointer
            void* userdata = lua_newuserdata( _lua, sizeof(osg::Object*));
            (*reinterpret_cast<osg::Object**>(userdata)) = object;

            luaL_getmetatable( _lua, "LuaScriptEngine.UnrefObject");
            lua_setmetatable( _lua, -2 );

            lua_settable(_lua, -3);

            // increment the reference count as the lua now will unreference it once it's finished with the userdata for the pointer
            object->ref();
        }

        std::string::size_type seperator = compoundClassName.find("::");
        std::string libraryName = (seperator==std::string::npos) ? object->libraryName() : compoundClassName.substr(0, seperator);
        std::string className = (seperator==std::string::npos) ? object->className() : compoundClassName.substr(seperator+2,std::string::npos);

        lua_pushstring(_lua, "libraryName"); lua_pushstring(_lua, libraryName.c_str()); lua_settable(_lua, -3);
        lua_pushstring(_lua, "className"); lua_pushstring(_lua, className.c_str()); lua_settable(_lua, -3);

        lua_pushstring(_lua, "compoundClassName"); lua_pushstring(_lua, compoundClassName.c_str()); lua_settable(_lua, -3);

        luaL_getmetatable(_lua, "LuaScriptEngine.Object");
        lua_setmetatable(_lua, -2);
    }
    else
    {
        lua_pushnil(_lua);
    }
}

void LuaScriptEngine::assignClosure(const char* name, lua_CFunction fn) const
{
    lua_pushstring(_lua, name);
    lua_pushlightuserdata(_lua, const_cast<LuaScriptEngine*>(this));
    lua_pushcclosure(_lua, fn, 1);
    lua_settable(_lua, -3);
}
