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
#include <osg/observer_ptr>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

using namespace lua;




class LuaCallbackObject : public osg::CallbackObject
{
public:
    LuaCallbackObject(const std::string& methodName, const LuaScriptEngine* lse, int ref):_lse(lse),_ref(ref)
    {
        setName(methodName);
    }

    virtual bool run(osg::Object* object, osg::Parameters& inputParameters, osg::Parameters& outputParameters) const
    {
        if (!_lse)
        {
            OSG_NOTICE << "Warning: Ignoring call to Lua by an expired callback" << std::endl;
            return false;
        }
        
        // a strong reference is necessary as the lua call might trigger deletion of the LuaScriptEngine object
        // avoid overhead by observer_ptr<>::lock as a race on run/destruction never is valid
        osg::ref_ptr<const LuaScriptEngine> lse(_lse.get());

        int topBeforeCall = lua_gettop(lse->getLuaState());

        lua_rawgeti(lse->getLuaState(), LUA_REGISTRYINDEX, _ref);

        int numInputs = 1;
        lse->pushParameter(object);

        for(osg::Parameters::iterator itr = inputParameters.begin();
            itr != inputParameters.end();
            ++itr)
        {
            lse->pushParameter(itr->get());
            ++numInputs;
        }

        if (lua_pcall(lse->getLuaState(), numInputs, LUA_MULTRET,0)!=0)
        {
            OSG_NOTICE<<"Lua error : "<<lua_tostring(lse->getLuaState(), -1)<<std::endl;
            return false;
        }

        int topAfterCall = lua_gettop(lse->getLuaState());
        int numReturns = topAfterCall-topBeforeCall;
        for(int i=1; i<=numReturns; ++i)
        {
            outputParameters.insert(outputParameters.begin(), lse->popParameterObject());
        }
        return true;
    }

    int getRef() const { return _ref; }

protected:

    osg::observer_ptr<const LuaScriptEngine> _lse;
    int _ref;
};


static int getProperty(lua_State * _lua)
{
    const LuaScriptEngine* lse = reinterpret_cast<const LuaScriptEngine*>(lua_topointer(_lua, lua_upvalueindex(1)));

    int n = lua_gettop(_lua);    /* number of arguments */
    if (n==2 && lua_type(_lua, 1)==LUA_TTABLE)
    {
        if (lua_type(_lua, 2)==LUA_TSTRING)
        {
            std::string propertyName = lua_tostring(_lua, 2);
            osg::Object* object  = lse->getObjectFromTable<osg::Object>(1);

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
    if (n==3 && lua_type(_lua, 1)==LUA_TTABLE)
    {
        if (lua_type(_lua, 2)==LUA_TSTRING)
        {
            std::string propertyName = lua_tostring(_lua, 2);
            osg::Object* object  = lse->getObjectFromTable<osg::Object>(1);

            return lse->setPropertyFromStack(object, propertyName);
        }
    }

    OSG_NOTICE<<"Warning: Lua setProperty() not matched"<<std::endl;
    return 0;
}

//////////////////////////////////////////////////////////////////////////////////////
//
//  Vector container support
//
static int getContainerProperty(lua_State * _lua)
{
    const LuaScriptEngine* lse = reinterpret_cast<const LuaScriptEngine*>(lua_topointer(_lua, lua_upvalueindex(1)));

    int n = lua_gettop(_lua);    /* number of arguments */
    if (n==2 && lua_type(_lua, 1)==LUA_TTABLE)
    {
        if (lua_type(_lua, 2)==LUA_TSTRING)
        {
            std::string propertyName = lua_tostring(_lua, 2);
            osg::Object* object  = lse->getObjectFromTable<osg::Object>(1);
            std::string containerPropertyName = lse->getStringFromTable(1,"containerPropertyName");

            return lse->pushPropertyToStack(object, propertyName);
        }
        else if (lua_type(_lua, 2)==LUA_TNUMBER)
        {
            double index = lua_tonumber(_lua, 2);
            const osg::Object* object  = lse->getObjectFromTable<osg::Object>(1);
            std::string containerPropertyName = lse->getStringFromTable(1,"containerPropertyName");

            // check to see if Object "is a" vector
            osgDB::BaseSerializer::Type type;
            osgDB::BaseSerializer* bs = lse->getClassInterface().getSerializer(object, containerPropertyName, type);
            osgDB::VectorBaseSerializer* vs = dynamic_cast<osgDB::VectorBaseSerializer*>(bs);
            if (vs)
            {
                const void* dataPtr = vs->getElement(*object, (unsigned int) index);
                if (dataPtr)
                {
                    SerializerScratchPad valuesp(vs->getElementType(), dataPtr, vs->getElementSize());
                    return lse->pushDataToStack(&valuesp);
                }
                else
                {
                    lua_pushnil(_lua);
                    return 1;
                }
            }
        }
    }

    OSG_NOTICE<<"Warning: Lua getContainerProperty() not matched"<<std::endl;
    return 0;
}


static int setContainerProperty(lua_State* _lua)
{
    const LuaScriptEngine* lse = reinterpret_cast<const LuaScriptEngine*>(lua_topointer(_lua, lua_upvalueindex(1)));

    int n = lua_gettop(_lua);    /* number of arguments */
    if (n==3 && lua_type(_lua, 1)==LUA_TTABLE)
    {
        if (lua_type(_lua, 2)==LUA_TSTRING)
        {
            std::string propertyName = lua_tostring(_lua, 2);
            osg::Object* object  = lse->getObjectFromTable<osg::Object>(1);
            std::string containerPropertyName = lse->getStringFromTable(1,"containerPropertyName");

            return lse->setPropertyFromStack(object, propertyName);
        }
        else if (lua_type(_lua, 2)==LUA_TNUMBER)
        {
            double index = lua_tonumber(_lua, 2);

            osg::Object* object  = lse->getObjectFromTable<osg::Object>(1);
            std::string containerPropertyName = lse->getStringFromTable(1,"containerPropertyName");

            // check to see if Object "is a" vector
            osgDB::BaseSerializer::Type type;
            osgDB::BaseSerializer* bs = lse->getClassInterface().getSerializer(object, containerPropertyName, type);
            osgDB::VectorBaseSerializer* vs = dynamic_cast<osgDB::VectorBaseSerializer*>(bs);
            if (vs)
            {
                SerializerScratchPad ssp;
                lse->getDataFromStack(&ssp, vs->getElementType(), 3);
                {
                    vs->setElement(*object, (unsigned int) index, ssp.data);
                }
            }
            return 0;
        }
    }

    OSG_NOTICE<<"Warning: Lua setContainerProperty() not matched"<<std::endl;
    return 0;
}

static int getContainerSize(lua_State* _lua)
{
    const LuaScriptEngine* lse = reinterpret_cast<const LuaScriptEngine*>(lua_topointer(_lua, lua_upvalueindex(1)));
    int n = lua_gettop(_lua);    /* number of arguments */
    if (n<1 || lua_type(_lua, 1)!=LUA_TTABLE) return 0;

    osg::Object* object  = lse->getObjectFromTable<osg::Object>(1);
    std::string containerPropertyName = lse->getStringFromTable(1,"containerPropertyName");

    // check to see if Object "is a" vector
    osgDB::BaseSerializer::Type type;
    osgDB::BaseSerializer* bs = lse->getClassInterface().getSerializer(object, containerPropertyName, type);
    osgDB::VectorBaseSerializer* vs = dynamic_cast<osgDB::VectorBaseSerializer*>(bs);
    if (vs)
    {
        lua_pushinteger(lse->getLuaState(), vs->size(*object));
        return 1;
    }

    return 0;
}

static int callVectorClear(lua_State* _lua)
{
    const LuaScriptEngine* lse = reinterpret_cast<const LuaScriptEngine*>(lua_topointer(_lua, lua_upvalueindex(1)));
    int n = lua_gettop(_lua);    /* number of arguments */
    if (n<1 || lua_type(_lua, 1)!=LUA_TTABLE) return 0;

    osg::Object* object  = lse->getObjectFromTable<osg::Object>(1);
    std::string containerPropertyName = lse->getStringFromTable(1,"containerPropertyName");

    // check to see if Object "is a" vector
    osgDB::BaseSerializer::Type type;
    osgDB::BaseSerializer* bs = lse->getClassInterface().getSerializer(object, containerPropertyName, type);
    osgDB::VectorBaseSerializer* vs = dynamic_cast<osgDB::VectorBaseSerializer*>(bs);
    if (vs)
    {
        vs->clear(*object);
        return 0;
    }

    return 0;
}

static int callVectorResize(lua_State* _lua)
{
    const LuaScriptEngine* lse = reinterpret_cast<const LuaScriptEngine*>(lua_topointer(_lua, lua_upvalueindex(1)));
    int n = lua_gettop(_lua);    /* number of arguments */
    if (n<2 || lua_type(_lua, 1)!=LUA_TTABLE || lua_type(_lua, 2)!=LUA_TNUMBER) return 0;

    double numElements = lua_tonumber(lse->getLuaState(),2);
    osg::Object* object  = lse->getObjectFromTable<osg::Object>(1);
    std::string containerPropertyName = lse->getStringFromTable(1,"containerPropertyName");

    // check to see if Object "is a" vector
    osgDB::BaseSerializer::Type type;
    osgDB::BaseSerializer* bs = lse->getClassInterface().getSerializer(object, containerPropertyName, type);
    osgDB::VectorBaseSerializer* vs = dynamic_cast<osgDB::VectorBaseSerializer*>(bs);
    if (vs)
    {
        vs->resize(*object, static_cast<unsigned int>(numElements));
    }

    return 0;
}

static int callVectorReserve(lua_State* _lua)
{
    const LuaScriptEngine* lse = reinterpret_cast<const LuaScriptEngine*>(lua_topointer(_lua, lua_upvalueindex(1)));
    int n = lua_gettop(_lua);    /* number of arguments */
    if (n<2 || lua_type(_lua, 1)!=LUA_TTABLE || lua_type(_lua, 2)!=LUA_TNUMBER) return 0;

    double numElements = lua_tonumber(lse->getLuaState(),2);
    osg::Object* object  = lse->getObjectFromTable<osg::Object>(1);
    std::string containerPropertyName = lse->getStringFromTable(1,"containerPropertyName");

    // check to see if Object "is a" vector
    osgDB::BaseSerializer::Type type;
    osgDB::BaseSerializer* bs = lse->getClassInterface().getSerializer(object, containerPropertyName, type);
    osgDB::VectorBaseSerializer* vs = dynamic_cast<osgDB::VectorBaseSerializer*>(bs);
    if (vs)
    {
        vs->reserve(*object, static_cast<unsigned int>(numElements));
    }

    return 0;
}


static int callVectorAdd(lua_State* _lua)
{
    const LuaScriptEngine* lse = reinterpret_cast<const LuaScriptEngine*>(lua_topointer(_lua, lua_upvalueindex(1)));
    int n = lua_gettop(_lua);    /* number of arguments */
    if (n<2 || lua_type(_lua, 1)!=LUA_TTABLE) return 0;

    osg::Object* object  = lse->getObjectFromTable<osg::Object>(1);
    std::string containerPropertyName = lse->getStringFromTable(1,"containerPropertyName");

    // check to see if Object "is a" vector
    osgDB::BaseSerializer::Type type;
    osgDB::BaseSerializer* bs = lse->getClassInterface().getSerializer(object, containerPropertyName, type);
    osgDB::VectorBaseSerializer* vs = dynamic_cast<osgDB::VectorBaseSerializer*>(bs);
    if (vs)
    {
        SerializerScratchPad ssp;
        lse->getDataFromStack(&ssp, vs->getElementType(), 2);

        if (ssp.dataType==vs->getElementType())
        {
            vs->addElement(*object, ssp.data);
        }
        else
        {
            OSG_NOTICE<<"Failed to match table type"<<std::endl;
        }

    }

    return 0;
}

//////////////////////////////////////////////////////////////////////////////////////
//
//  Map container support
//
static int getMapProperty(lua_State * _lua)
{
    const LuaScriptEngine* lse = reinterpret_cast<const LuaScriptEngine*>(lua_topointer(_lua, lua_upvalueindex(1)));

    int n = lua_gettop(_lua);    /* number of arguments */
    if (n==2 && lua_type(_lua, 1)==LUA_TTABLE)
    {
        if (lua_type(_lua, 2)==LUA_TSTRING)
        {
            std::string propertyName = lua_tostring(_lua, 2);
            osg::Object* object  = lse->getObjectFromTable<osg::Object>(1);
            std::string containerPropertyName = lse->getStringFromTable(1,"containerPropertyName");

            return lse->pushPropertyToStack(object, propertyName);
        }
        else
        {
            const osg::Object* object  = lse->getObjectFromTable<osg::Object>(1);
            std::string containerPropertyName = lse->getStringFromTable(1,"containerPropertyName");

            // check to see if Object "is a" vector
            osgDB::BaseSerializer::Type type;
            osgDB::BaseSerializer* bs = lse->getClassInterface().getSerializer(object, containerPropertyName, type);
            osgDB::MapBaseSerializer* ms = dynamic_cast<osgDB::MapBaseSerializer*>(bs);

            if (ms)
            {
                SerializerScratchPad keysp;
                lse->getDataFromStack(&keysp, ms->getKeyType(),2);
                if (keysp.dataType==ms->getKeyType())
                {
                    const void* dataPtr = ms->getElement(*object, keysp.data);
                    if (dataPtr)
                    {
                        SerializerScratchPad valuesp(ms->getElementType(), dataPtr, ms->getElementSize());
                        return lse->pushDataToStack(&valuesp);
                    }
                    else
                    {
                        lua_pushnil(_lua);
                        return 1;
                    }

                   return 0;
                }
            }
        }
    }

    OSG_NOTICE<<"Warning: Lua getMapProperty() not matched"<<std::endl;
    return 0;
}


static int setMapProperty(lua_State* _lua)
{
    const LuaScriptEngine* lse = reinterpret_cast<const LuaScriptEngine*>(lua_topointer(_lua, lua_upvalueindex(1)));

    int n = lua_gettop(_lua);    /* number of arguments */

    if (n==3 && lua_type(_lua, 1)==LUA_TTABLE)
    {
        if (lua_type(_lua, 2)==LUA_TSTRING)
        {
            std::string propertyName = lua_tostring(_lua, 2);
            osg::Object* object  = lse->getObjectFromTable<osg::Object>(1);
            std::string containerPropertyName = lse->getStringFromTable(1,"containerPropertyName");

            return lse->setPropertyFromStack(object, propertyName);
        }
        else
        {
            osg::Object* object  = lse->getObjectFromTable<osg::Object>(1);
            std::string containerPropertyName = lse->getStringFromTable(1,"containerPropertyName");

            // check to see if Object "is a" vector
            osgDB::BaseSerializer::Type type;
            osgDB::BaseSerializer* bs = lse->getClassInterface().getSerializer(object, containerPropertyName, type);
            osgDB::MapBaseSerializer* ms = dynamic_cast<osgDB::MapBaseSerializer*>(bs);

            if (ms)
            {
                SerializerScratchPad keysp, valuesp;
                lse->getDataFromStack(&keysp, ms->getKeyType(),2);
                lse->getDataFromStack(&valuesp, ms->getElementType(),3);
                if (keysp.dataType==ms->getKeyType() && ms->getElementType()==valuesp.dataType)
                {
                    ms->setElement(*object, keysp.data, valuesp.data);
                    return 0;
                }
                else
                {
                    OSG_NOTICE<<"Warning: Lua setMapProperty() : Failed to matched map element "<<std::endl;
                    OSG_NOTICE<<"                                keysp.dataType="<<keysp.dataType<<std::endl;
                    OSG_NOTICE<<"                                valuesp.dataType="<<valuesp.dataType<<std::endl;
                    return 0;
                }

            }
        }
    }

    OSG_NOTICE<<"Warning: Lua setMapProperty() not matched"<<std::endl;
    return 0;
}

static int callMapClear(lua_State* _lua)
{
    const LuaScriptEngine* lse = reinterpret_cast<const LuaScriptEngine*>(lua_topointer(_lua, lua_upvalueindex(1)));
    int n = lua_gettop(_lua);    /* number of arguments */
    if (n<1 || lua_type(_lua, 1)!=LUA_TTABLE) return 0;

    osg::Object* object  = lse->getObjectFromTable<osg::Object>(1);
    std::string containerPropertyName = lse->getStringFromTable(1,"containerPropertyName");

    // check to see if Object "is a" vector
    osgDB::BaseSerializer::Type type;
    osgDB::BaseSerializer* bs = lse->getClassInterface().getSerializer(object, containerPropertyName, type);
    osgDB::MapBaseSerializer* ms = dynamic_cast<osgDB::MapBaseSerializer*>(bs);
    if (ms)
    {
        ms->clear(*object);
        return 0;
    }

    return 0;
}

static int getMapSize(lua_State* _lua)
{
    const LuaScriptEngine* lse = reinterpret_cast<const LuaScriptEngine*>(lua_topointer(_lua, lua_upvalueindex(1)));
    int n = lua_gettop(_lua);    /* number of arguments */
    if (n<1 || lua_type(_lua, 1)!=LUA_TTABLE) return 0;

    osg::Object* object  = lse->getObjectFromTable<osg::Object>(1);
    std::string containerPropertyName = lse->getStringFromTable(1,"containerPropertyName");

    // check to see if Object "is a" vector
    osgDB::BaseSerializer::Type type;
    osgDB::BaseSerializer* bs = lse->getClassInterface().getSerializer(object, containerPropertyName, type);
    osgDB::MapBaseSerializer* ms = dynamic_cast<osgDB::MapBaseSerializer*>(bs);
    if (ms)
    {
        lua_pushinteger(lse->getLuaState(), ms->size(*object));
        return 1;
    }

    return 0;
}


static int createMapIterator(lua_State* _lua)
{
    const LuaScriptEngine* lse = reinterpret_cast<const LuaScriptEngine*>(lua_topointer(_lua, lua_upvalueindex(1)));
    int n = lua_gettop(_lua);    /* number of arguments */
    if (n<1 || lua_type(_lua, 1)!=LUA_TTABLE) return 0;

    osg::Object* object  = lse->getObjectFromTable<osg::Object>(1);
    std::string containerPropertyName = lse->getStringFromTable(1,"containerPropertyName");

    // check to see if Object "is a" vector
    osgDB::BaseSerializer::Type type;
    osgDB::BaseSerializer* bs = lse->getClassInterface().getSerializer(object, containerPropertyName, type);
    osgDB::MapBaseSerializer* ms = dynamic_cast<osgDB::MapBaseSerializer*>(bs);
    if (ms)
    {
        lse->pushObject(ms->createIterator(*object));
        return 1;
    }

    return 0;
}

static int createMapReverseIterator(lua_State* _lua)
{
    const LuaScriptEngine* lse = reinterpret_cast<const LuaScriptEngine*>(lua_topointer(_lua, lua_upvalueindex(1)));
    int n = lua_gettop(_lua);    /* number of arguments */
    if (n<1 || lua_type(_lua, 1)!=LUA_TTABLE) return 0;

    osg::Object* object  = lse->getObjectFromTable<osg::Object>(1);
    std::string containerPropertyName = lse->getStringFromTable(1,"containerPropertyName");

    // check to see if Object "is a" vector
    osgDB::BaseSerializer::Type type;
    osgDB::BaseSerializer* bs = lse->getClassInterface().getSerializer(object, containerPropertyName, type);
    osgDB::MapBaseSerializer* ms = dynamic_cast<osgDB::MapBaseSerializer*>(bs);
    if (ms)
    {
        lse->pushObject(ms->createReverseIterator(*object));
        return 1;
    }

    return 0;
}

//////////////////////////////////////////////////////////////////////////////////////
//
//  MapIteratorObject support
//
static int callMapIteratorAdvance(lua_State* _lua)
{
    const LuaScriptEngine* lse = reinterpret_cast<const LuaScriptEngine*>(lua_topointer(_lua, lua_upvalueindex(1)));
    if (lua_gettop(_lua)<1 || lua_type(_lua, 1)!=LUA_TTABLE) return 0;
    osgDB::MapIteratorObject* mio  = lse->getObjectFromTable<osgDB::MapIteratorObject>(1);
    if (mio)
    {
        lua_pushboolean(lse->getLuaState(), mio->advance());
        return 1;
    }

    return 0;
}

static int callMapIteratorValid(lua_State* _lua)
{
    const LuaScriptEngine* lse = reinterpret_cast<const LuaScriptEngine*>(lua_topointer(_lua, lua_upvalueindex(1)));
    if (lua_gettop(_lua)<1 || lua_type(_lua, 1)!=LUA_TTABLE) return 0;
    osgDB::MapIteratorObject* mio  = lse->getObjectFromTable<osgDB::MapIteratorObject>(1);
    if (mio)
    {
        lua_pushboolean(lse->getLuaState(), mio->valid());
        return 1;
    }

    return 0;
}

static int getMapIteratorKey(lua_State* _lua)
{
    const LuaScriptEngine* lse = reinterpret_cast<const LuaScriptEngine*>(lua_topointer(_lua, lua_upvalueindex(1)));
    if (lua_gettop(_lua)<1 || lua_type(_lua, 1)!=LUA_TTABLE) return 0;
    osgDB::MapIteratorObject* mio  = lse->getObjectFromTable<osgDB::MapIteratorObject>(1);
    if (mio)
    {
        const void* dataPtr = mio->getKey();
        if (dataPtr)
        {
            SerializerScratchPad valuesp(mio->getKeyType(), dataPtr, mio->getKeySize());
            return lse->pushDataToStack(&valuesp);
        }
        else
        {
            lua_pushnil(_lua);
            return 1;
        }
    }

    return 0;
}

static int getMapIteratorElement(lua_State* _lua)
{
    const LuaScriptEngine* lse = reinterpret_cast<const LuaScriptEngine*>(lua_topointer(_lua, lua_upvalueindex(1)));
    if (lua_gettop(_lua)<1 || lua_type(_lua, 1)!=LUA_TTABLE) return 0;
    osgDB::MapIteratorObject* mio  = lse->getObjectFromTable<osgDB::MapIteratorObject>(1);
    if (mio)
    {
        const void* dataPtr = mio->getElement();
        if (dataPtr)
        {
            SerializerScratchPad valuesp(mio->getElementType(), dataPtr, mio->getElementSize());
            return lse->pushDataToStack(&valuesp);
        }
        else
        {
            lua_pushnil(_lua);
            return 1;
        }
    }
    OSG_NOTICE<<"getMapIteratorElement failed. "<<std::endl;
    return 0;
}


static int setMapIteratorElement(lua_State* _lua)
{
    const LuaScriptEngine* lse = reinterpret_cast<const LuaScriptEngine*>(lua_topointer(_lua, lua_upvalueindex(1)));
    if (lua_gettop(_lua)<2 || lua_type(_lua, 1)!=LUA_TTABLE) return 0;
    osgDB::MapIteratorObject* mio  = lse->getObjectFromTable<osgDB::MapIteratorObject>(1);
    if (mio)
    {
        SerializerScratchPad valuesp;
        lse->getDataFromStack(&valuesp, mio->getElementType(), 2);

        if (mio->getElementType()==valuesp.dataType)
        {
            mio->setElement(valuesp.data);
            return 0;
        }
        else
        {
            OSG_NOTICE<<"Warning: Lua setMapIteratorElement() : Failed to matched map element type, valuesp.dataType="<<valuesp.dataType<<std::endl;
            return 0;
        }
    }

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////
//
//  StateSet support
//
static int convertStringToStateAttributeValue(const std::string& valueString, osg::StateAttribute::OverrideValue defaultValue, bool& setOnOff)
{
    osg::StateAttribute::OverrideValue value=defaultValue;

    if (valueString.find("ON")!=std::string::npos) { value = osg::StateAttribute::ON; setOnOff = true; }
    if (valueString.find("OFF")!=std::string::npos) { value = osg::StateAttribute::OFF; setOnOff = true; }

    if (valueString.find("OVERRIDE")!=std::string::npos) value = value | osg::StateAttribute::OVERRIDE;
    if (valueString.find("PROTECTED")!=std::string::npos) value = value | osg::StateAttribute::PROTECTED;
    if (valueString.find("INHERIT")!=std::string::npos) value = value | osg::StateAttribute::INHERIT;
    return value;
}

static std::string convertStateAttributeValueToString(unsigned int value, bool withOnOffCheck)
{
    std::string valueString;
    if (withOnOffCheck)
    {
        if ((value&osg::StateAttribute::ON)!=0) { if (!valueString.empty()) valueString.append(", "); valueString.append("ON"); }
        else { if (!valueString.empty()) valueString.append(", "); valueString.append("OFF"); }
    }
    if ((value&osg::StateAttribute::OVERRIDE)!=0) { if (!valueString.empty()) valueString.append(", "); valueString.append("OVERRIDE"); }
    if ((value&osg::StateAttribute::PROTECTED)!=0) { if (!valueString.empty()) valueString.append(", "); valueString.append("PROTECTED"); }
    if ((value&osg::StateAttribute::INHERIT)!=0) { if (!valueString.empty()) valueString.append(", "); valueString.append("INHERIT"); }
    return valueString;
}

static int callStateSetSet(lua_State* _lua)
{
    const LuaScriptEngine* lse = reinterpret_cast<const LuaScriptEngine*>(lua_topointer(_lua, lua_upvalueindex(1)));
    int n = lua_gettop(_lua);    /* number of arguments */
    if (n<2 || lua_type(_lua, 1)!=LUA_TTABLE) return 0;

    osg::StateSet* stateset  = lse->getObjectFromTable<osg::StateSet>(1);
    if (!stateset)
    {
        OSG_NOTICE<<"Warning: StateSet:add() can only be called on a StateSet"<<std::endl;
        return 0;
    }

    if (lua_type(_lua,2)==LUA_TTABLE)
    {
        osg::Object* po  = lse->getObjectFromTable<osg::Object>(2);
        osg::StateAttribute* sa = dynamic_cast<osg::StateAttribute*>(po);
        osg::Uniform* uniform = dynamic_cast<osg::Uniform*>(po);

        osg::StateAttribute::OverrideValue value=osg::StateAttribute::ON;
        bool setOnOff = false;
        if (n>=3 && lua_type(_lua,3)==LUA_TSTRING)
        {
            value = convertStringToStateAttributeValue(lua_tostring(_lua, 3), value, setOnOff);
        }

        if (sa)
        {
            if (setOnOff)
            {
                if (sa->isTextureAttribute()) stateset->setTextureAttributeAndModes(0, sa, value);
                else  stateset->setAttributeAndModes(sa, value);
            }
            else
            {
                if (sa->isTextureAttribute()) stateset->setTextureAttribute(0, sa, value);
                else  stateset->setAttribute(sa, value);
            }
            return 0;
        }
        else if (uniform)
        {
            stateset->addUniform(uniform, value);
            return 0;
        }
    }
    else if (lua_type(_lua,2)==LUA_TNUMBER)
    {
        double index = lua_tonumber(_lua, 2);
        if (n>=3)
        {
            if (lua_type(_lua,3)==LUA_TTABLE)
            {
                osg::Object* po  = lse->getObjectFromTable<osg::Object>(3);
                osg::StateAttribute* sa = dynamic_cast<osg::StateAttribute*>(po);

                osg::StateAttribute::OverrideValue value=osg::StateAttribute::ON;
                bool setOnOff = false;
                if (n>=4 && lua_type(_lua,4)==LUA_TSTRING)
                {
                    value = convertStringToStateAttributeValue(lua_tostring(_lua, 4), value, setOnOff);
                }

                if (sa)
                {
                    if (setOnOff)
                    {
                        stateset->setTextureAttributeAndModes(static_cast<unsigned int>(index), sa, value);
                    }
                    else
                    {
                        stateset->setTextureAttribute(static_cast<unsigned int>(index), sa, value);
                    }
                    return 0;
                }
            }
            else if (lua_type(_lua,3)==LUA_TSTRING)
            {
                std::string modeString = lua_tostring(_lua, 3);
                GLenum mode = lse->lookUpGLenumValue(modeString);

                osg::StateAttribute::OverrideValue value=osg::StateAttribute::ON;
                bool setOnOff = false;
                if (n>=4 && lua_type(_lua,4)==LUA_TSTRING)
                {
                    value = convertStringToStateAttributeValue(lua_tostring(_lua, 4), value, setOnOff);
                }
                stateset->setTextureMode(static_cast<unsigned int>(index), mode, value);
                return 0;
            }
        }
    }
    else if (lua_type(_lua,2)==LUA_TSTRING)
    {
        std::string key = lua_tostring(_lua, 2);
        GLenum mode = lse->lookUpGLenumValue(key);
        if (n>=3)
        {
            if (mode)
            {
                osg::StateAttribute::OverrideValue value=osg::StateAttribute::ON;
                bool setOnOff = false;
                if (lua_type(_lua,3)==LUA_TSTRING)
                {
                    value = convertStringToStateAttributeValue(lua_tostring(_lua, 3), value, setOnOff);
                }

                stateset->setMode(mode, value);
                return 0;
            }
            else
            {
                std::string value;
                if (lua_type(_lua,3)==LUA_TSTRING)
                {
                    value = lua_tostring(_lua, 3);
                }
                stateset->setDefine(key, value);
            }
        }
        else
        {
            if (mode)
            {
                osg::StateAttribute::OverrideValue value=osg::StateAttribute::ON;
                stateset->setMode(mode, value);
                return 0;
            }
            else
            {
                stateset->setDefine(key);
            }
        }
    }

    OSG_NOTICE<<"Warning: StateSet:set() inappropriate parameters, use form:"<<std::endl;
    OSG_NOTICE<<"   StateSet:set(modestring [,value=\"ON,OFF,OVERRIDE,PROTECTED\"]); "<<std::endl;
    OSG_NOTICE<<"   StateSet:set(uniform [,value=\"ON,OFF,OVERRIDE,PROTECTED\"]); "<<std::endl;
    OSG_NOTICE<<"   StateSet:set(attribute [,value=\"ON,OFF,OVERRIDE,PROTECTED\"]); "<<std::endl;
    OSG_NOTICE<<"   StateSet:set(textureUnit, textureAttribute [,value=\"ON,OFF,OVERRIDE,PROTECTED\"]); "<<std::endl;
    return 0;
}

static int callStateSetGet(lua_State* _lua)
{
    const LuaScriptEngine* lse = reinterpret_cast<const LuaScriptEngine*>(lua_topointer(_lua, lua_upvalueindex(1)));
    int n = lua_gettop(_lua);    /* number of arguments */
    if (n<2 || lua_type(_lua, 1)!=LUA_TTABLE) return 0;

    osg::StateSet* stateset  = lse->getObjectFromTable<osg::StateSet>(1);
    if (!stateset)
    {
        OSG_NOTICE<<"Warning: StateSet:get() can only be called on a StateSet"<<std::endl;
        return 0;
    }

    if (lua_type(_lua,2)==LUA_TNUMBER)
    {
        if (n<3)
        {
            OSG_NOTICE<<"Warning: StateSet:get() must be in form get(textureUnit, ClassName|ModeName|ObjectName)"<<std::endl;
            return 0;
        }

        unsigned int index = static_cast<unsigned int>(lua_tonumber(_lua, 2));
        if (lua_type(_lua,3)==LUA_TTABLE)
        {
            osg::Object* po  = lse->getObjectFromTable<osg::Object>(3);
            osg::StateAttribute* sa = dynamic_cast<osg::StateAttribute*>(po);
            if (sa && sa->isTextureAttribute())
            {
                if (stateset->getTextureAttributeList().size()>index)
                {
                    const osg::StateSet::AttributeList& al = stateset->getTextureAttributeList()[index];
                    for(osg::StateSet::AttributeList::const_iterator itr = al.begin();
                        itr != al.end();
                        ++itr)
                    {
                        if (itr->second.first==sa)
                        {
                            lua_newtable(_lua);
                            lua_pushstring(_lua, "attribute"); lse->pushObject(itr->second.first.get()); lua_settable(_lua, -3);
                            lua_pushstring(_lua, "value"); lua_pushstring(_lua, convertStateAttributeValueToString(itr->second.second, false).c_str()); lua_settable(_lua, -3);
                            return 1;
                        }
                    }
                }
                OSG_NOTICE<<"Warning: StateSet:get() Could not find attribute : "<<sa->className()<<std::endl;
                lua_pushnil(_lua);
                return 1;
            }
            lua_pushnil(_lua);
            return 1;
        }
        else if (lua_type(_lua,3)==LUA_TSTRING)
        {
            std::string value = lua_tostring(_lua, 3);
            // need to look for attribute of mode with specified value
            if (stateset->getTextureAttributeList().size()>index)
            {
                const osg::StateSet::AttributeList& al = stateset->getTextureAttributeList()[index];
                for(osg::StateSet::AttributeList::const_iterator itr = al.begin();
                    itr != al.end();
                    ++itr)
                {
                    if (value == itr->second.first->className() ||
                        value == itr->second.first->getName())
                    {
                        lua_newtable(_lua);
                        lua_pushstring(_lua, "attribute"); lse->pushObject(itr->second.first.get()); lua_settable(_lua, -3);
                        lua_pushstring(_lua, "value"); lua_pushstring(_lua, convertStateAttributeValueToString(itr->second.second, false).c_str()); lua_settable(_lua, -3);
                        return 1;
                    }
                }
            }
            if (stateset->getTextureModeList().size()>index)
            {
                osg::StateAttribute::GLMode mode = lse->lookUpGLenumValue(value);
                const osg::StateSet::ModeList& ml = stateset->getTextureModeList()[index];
                for(osg::StateSet::ModeList::const_iterator itr = ml.begin();
                    itr != ml.end();
                    ++itr)
                {
                    if (mode == itr->first)
                    {
                        lua_pushstring(_lua, convertStateAttributeValueToString(itr->second, true).c_str());
                        return 1;
                    }
                }
            }

            OSG_NOTICE<<"Warning: StateSet:get() Could not find attribute : "<<value<<std::endl;
            lua_pushnil(_lua);
            return 1;
        }
    }
    else if (lua_type(_lua,2)==LUA_TTABLE)
    {
        osg::Object* po  = lse->getObjectFromTable<osg::Object>(2);
        osg::StateAttribute* sa = dynamic_cast<osg::StateAttribute*>(po);
        osg::Uniform* uniform = dynamic_cast<osg::Uniform*>(po);

        if (sa && sa->isTextureAttribute() && stateset->getTextureAttributeList().size()>0)
        {
            const osg::StateSet::AttributeList& al = stateset->getTextureAttributeList()[0];
            for(osg::StateSet::AttributeList::const_iterator itr = al.begin();
                itr != al.end();
                ++itr)
            {
                if (itr->second.first==sa)
                {
                    lua_newtable(_lua);
                    lua_pushstring(_lua, "attribute"); lse->pushObject(itr->second.first.get()); lua_settable(_lua, -3);
                    lua_pushstring(_lua, "value"); lua_pushstring(_lua, convertStateAttributeValueToString(itr->second.second, false).c_str()); lua_settable(_lua, -3);
                    return 1;
                }
            }
            OSG_NOTICE<<"Warning: StateSet:get("<<sa->className()<<") Could not find attribute"<<std::endl;
            lua_pushnil(_lua);
            return 1;
        }
        else if (sa)
        {
            const osg::StateSet::AttributeList& al = stateset->getAttributeList();
            for(osg::StateSet::AttributeList::const_iterator itr = al.begin();
                itr != al.end();
                ++itr)
            {
                if (itr->second.first==sa)
                {
                    lua_newtable(_lua);
                    lua_pushstring(_lua, "attribute"); lse->pushObject(itr->second.first.get()); lua_settable(_lua, -3);
                    lua_pushstring(_lua, "value"); lua_pushstring(_lua, convertStateAttributeValueToString(itr->second.second, false).c_str()); lua_settable(_lua, -3);
                    return 1;
                }
            }
            OSG_NOTICE<<"Warning: StateSet:get("<<sa->className()<<") Could not find attribute"<<std::endl;
            lua_pushnil(_lua);
            return 1;
        }
        else if (uniform)
        {
            const osg::StateSet::UniformList& ul = stateset->getUniformList();
            for(osg::StateSet::UniformList::const_iterator itr = ul.begin();
                itr != ul.end();
                ++itr)
            {
                if (itr->second.first==uniform)
                {
                    lua_newtable(_lua);
                    lua_pushstring(_lua, "attribute"); lse->pushObject(itr->second.first.get()); lua_settable(_lua, -3);
                    lua_pushstring(_lua, "value"); lua_pushstring(_lua, convertStateAttributeValueToString(itr->second.second, false).c_str()); lua_settable(_lua, -3);
                    return 1;
                }
            }
            OSG_NOTICE<<"Warning: StateSet:get("<<sa->className()<<") Could not find uniform"<<std::endl;
            lua_pushnil(_lua);
            return 1;
        }
    }
    else if (lua_type(_lua,2)==LUA_TSTRING)
    {
        std::string value = lua_tostring(_lua, 2);
        const osg::StateSet::AttributeList& al = stateset->getAttributeList();
        for(osg::StateSet::AttributeList::const_iterator itr = al.begin();
            itr != al.end();
            ++itr)
        {
            if (value == itr->second.first->className() ||
                value == itr->second.first->getName())
            {
                lua_newtable(_lua);
                lua_pushstring(_lua, "attribute"); lse->pushObject(itr->second.first.get()); lua_settable(_lua, -3);
                lua_pushstring(_lua, "value"); lua_pushstring(_lua, convertStateAttributeValueToString(itr->second.second, false).c_str()); lua_settable(_lua, -3);
                return 1;
            }
        }

        const osg::StateSet::UniformList& ul = stateset->getUniformList();
        for(osg::StateSet::UniformList::const_iterator itr = ul.begin();
            itr != ul.end();
            ++itr)
        {
            if (value == itr->second.first->className() ||
                value == itr->second.first->getName())
            {
                lua_newtable(_lua);
                lua_pushstring(_lua, "attribute"); lse->pushObject(itr->second.first.get()); lua_settable(_lua, -3);
                lua_pushstring(_lua, "value"); lua_pushstring(_lua, convertStateAttributeValueToString(itr->second.second, false).c_str()); lua_settable(_lua, -3);
                return 1;
            }
        }

        osg::StateAttribute::GLMode mode = lse->lookUpGLenumValue(value);
        const osg::StateSet::ModeList& ml = stateset->getModeList();
        for(osg::StateSet::ModeList::const_iterator itr = ml.begin();
            itr != ml.end();
            ++itr)
        {
            if (mode == itr->first)
            {
                lua_pushstring(_lua, convertStateAttributeValueToString(itr->second, true).c_str());
                return 1;
            }
        }


        const osg::StateSet::DefineList& dl = stateset->getDefineList();
        for(osg::StateSet::DefineList::const_iterator itr = dl.begin();
            itr != dl.end();
            ++itr)
        {
            if (value == itr->first)
            {
                lua_pushstring(_lua, itr->second.first.c_str());
                return 1;
            }
        }

        OSG_NOTICE<<"Warning: StateSet:get("<<value<<") Could not find matching mode or attribute"<<std::endl;
        lua_pushnil(_lua);
        return 1;
    }

    OSG_NOTICE<<"Warning: StateSet:get() inappropriate parameters, use form:"<<std::endl;
    OSG_NOTICE<<"   StateSet:get(modestring); "<<std::endl;
    OSG_NOTICE<<"   StateSet:get(uniformName); "<<std::endl;
    OSG_NOTICE<<"   StateSet:get(attributeNameOrClassType); "<<std::endl;
    OSG_NOTICE<<"   StateSet:get(textureUnit, textureNameOrClassType); "<<std::endl;

    lua_pushnil(_lua);
    return 1;
}

static int callStateSetRemove(lua_State* _lua)
{
    const LuaScriptEngine* lse = reinterpret_cast<const LuaScriptEngine*>(lua_topointer(_lua, lua_upvalueindex(1)));
    int n = lua_gettop(_lua);    /* number of arguments */
    if (n<2 || lua_type(_lua, 1)!=LUA_TTABLE) return 0;

    osg::StateSet* stateset  = lse->getObjectFromTable<osg::StateSet>(1);
    if (!stateset)
    {
            OSG_NOTICE<<"Warning: StateSet:remove() can only be called on a StateSet"<<std::endl;
            return 0;
    }

    if (lua_type(_lua,2)==LUA_TNUMBER)
    {
        if (n<3)
        {
            OSG_NOTICE<<"Warning: StateSet:remove() must be in form remove(textureUnit, textureAttribute)"<<std::endl;
            return 0;
        }

        unsigned int index = static_cast<unsigned int>(lua_tonumber(_lua, 2));
        if (lua_type(_lua,3)==LUA_TTABLE)
        {
            osg::Object* po  = lse->getObjectFromTable<osg::Object>(3);
            osg::StateAttribute* sa = dynamic_cast<osg::StateAttribute*>(po);
            stateset->removeTextureAttribute(static_cast<unsigned int>(index), sa);
            return 0;
        }
        else if (lua_type(_lua,3)==LUA_TSTRING)
        {
            std::string value = lua_tostring(_lua, 3);
            if (stateset->getTextureAttributeList().size()>index)
            {
                const osg::StateSet::AttributeList& al = stateset->getTextureAttributeList()[index];
                for(osg::StateSet::AttributeList::const_iterator itr = al.begin();
                    itr != al.end();
                    ++itr)
                {
                    if (value == itr->second.first->className() ||
                        value == itr->second.first->getName())
                    {
                        stateset->removeTextureAttribute(index, itr->second.first.get());
                        return 0;
                    }
                }
            }

            if (stateset->getTextureModeList().size()>index)
            {
                osg::StateAttribute::GLMode mode = lse->lookUpGLenumValue(value);
                const osg::StateSet::ModeList& ml = stateset->getTextureModeList()[static_cast<unsigned int>(index)];
                for(osg::StateSet::ModeList::const_iterator itr = ml.begin();
                    itr != ml.end();
                    ++itr)
                {
                    if (mode == itr->first)
                    {
                        stateset->removeTextureMode(static_cast<unsigned int>(index), mode);
                        return 1;
                    }
                }
            }

            OSG_NOTICE<<"Warning: StateSet:remove("<<index<<", "<<value<<") could not find entry to remove."<<std::endl;
            return 0;
        }
    }
    else if (lua_type(_lua,2)==LUA_TTABLE)
    {
        osg::Object* po  = lse->getObjectFromTable<osg::Object>(2);
        osg::StateAttribute* sa = dynamic_cast<osg::StateAttribute*>(po);
        osg::Uniform* uniform = dynamic_cast<osg::Uniform*>(po);

        if (sa && sa->isTextureAttribute())
        {
            stateset->removeTextureAttribute(0, sa);
            return 0;
        }
        else if (sa)
        {
            stateset->removeAttribute(sa);
            return 0;
        }
        else if (uniform)
        {
            stateset->removeUniform(uniform);
            return 0;
        }
    }
    else if (lua_type(_lua,2)==LUA_TSTRING)
    {
        std::string value = lua_tostring(_lua, 2);
        const osg::StateSet::AttributeList& al = stateset->getAttributeList();
        for(osg::StateSet::AttributeList::const_iterator itr = al.begin();
            itr != al.end();
            ++itr)
        {
            if (value == itr->second.first->className() ||
                value == itr->second.first->getName())
            {
                stateset->removeAttribute(itr->second.first.get());
                return 0;
            }
        }

        const osg::StateSet::UniformList& ul = stateset->getUniformList();
        for(osg::StateSet::UniformList::const_iterator itr = ul.begin();
            itr != ul.end();
            ++itr)
        {
            if (value == itr->second.first->className() ||
                value == itr->second.first->getName())
            {
                stateset->removeUniform(itr->second.first.get());
                return 0;
            }
        }

        const osg::StateSet::DefineList& dl = stateset->getDefineList();
        for(osg::StateSet::DefineList::const_iterator itr = dl.begin();
            itr != dl.end();
            ++itr)
        {
            if (value == itr->first)
            {
                stateset->removeDefine(value);
                return 0;
            }
        }

        osg::StateAttribute::GLMode mode = lse->lookUpGLenumValue(value);
        const osg::StateSet::ModeList& ml = stateset->getModeList();
        for(osg::StateSet::ModeList::const_iterator itr = ml.begin();
            itr != ml.end();
            ++itr)
        {
            if (mode == itr->first)
            {
                stateset->removeMode(mode);
                return 1;
            }
        }


        OSG_NOTICE<<"Warning: StateSet:remove("<<value<<") could not find entry to remove."<<std::endl;
        return 0;
    }

    OSG_NOTICE<<"Warning: StateSet:remove() inappropriate parameters, use form:"<<std::endl;
    OSG_NOTICE<<"   StateSet:remove(uniform); "<<std::endl;
    OSG_NOTICE<<"   StateSet:remove(attribute); "<<std::endl;
    OSG_NOTICE<<"   StateSet:remove(textureUnit, textureAttribute); "<<std::endl;
    return 0;
}

//////////////////////////////////////////////////////////////////////////////////////
//
//  Image calling support
//
static int callImageAllocate(lua_State* _lua)
{
    const LuaScriptEngine* lse = reinterpret_cast<const LuaScriptEngine*>(lua_topointer(_lua, lua_upvalueindex(1)));
    int n = lua_gettop(_lua);    /* number of arguments */
    if (n<1 || lua_type(_lua, 1)!=LUA_TTABLE) return 0;

    osg::Image* image  = lse->getObjectFromTable<osg::Image>(1);
    if (!image)
    {
        OSG_NOTICE<<"Warning: Image:allocate() can only be called on a Image"<<std::endl;
        return 0;
    }

    int s=0, t=0, r=0;
    GLenum pixelFormat=0;
    GLenum dataType = 0;
    int packing = 1;

    if (n>=2 && lua_isnumber(_lua, 2)) s = static_cast<int>(lua_tonumber(_lua, 2));
    if (n>=3 && lua_isnumber(_lua, 3)) t = static_cast<int>(lua_tonumber(_lua, 3));
    if (n>=4 && lua_isnumber(_lua, 4)) r = static_cast<int>(lua_tonumber(_lua, 4));

    if (n>=5)
    {
        if (lua_isnumber(_lua, 5)) pixelFormat = static_cast<int>(lua_tonumber(_lua, 5));
        else if (lua_isstring(_lua, 5))
        {
            pixelFormat = lse->lookUpGLenumValue(lua_tostring(_lua,5));
        }
    }

    if (n>=6)
    {
        if (lua_isnumber(_lua, 6)) dataType = static_cast<int>(lua_tonumber(_lua, 6));
        else if (lua_isstring(_lua, 6))
        {
            dataType = lse->lookUpGLenumValue(lua_tostring(_lua,6));
        }
    }

    if (n>=7)
    {
        if (lua_isnumber(_lua, 7)) packing = static_cast<int>(lua_tonumber(_lua, 7));
    }


    if (s<=0 || t<=0 || r<=0 || pixelFormat==0 || dataType==0)
    {
        OSG_NOTICE<<"Warning: Cannot not image:allocator("<<s<<", "<<t<<", "<<r<<", "<<pixelFormat<<", "<<dataType<<") a zero sized image, use non zero, positive values for s,t,r, pixelFormat and dataType."<<std::endl;
        return 0;
    }

    image->allocateImage(s,t,r,pixelFormat,dataType,packing);

    return 0;
}

static int callImageS(lua_State* _lua)
{
    const LuaScriptEngine* lse = reinterpret_cast<const LuaScriptEngine*>(lua_topointer(_lua, lua_upvalueindex(1)));
    int n = lua_gettop(_lua);    /* number of arguments */
    if (n<1 || lua_type(_lua, 1)!=LUA_TTABLE) return 0;

    osg::Image* image  = lse->getObjectFromTable<osg::Image>(1);
    if (!image)
    {
        OSG_NOTICE<<"Warning: Image:s() can only be called on a Image"<<std::endl;
        return 0;
    }

    lua_pushinteger(_lua, image->s());

    return 1;
}

static int callImageT(lua_State* _lua)
{
    const LuaScriptEngine* lse = reinterpret_cast<const LuaScriptEngine*>(lua_topointer(_lua, lua_upvalueindex(1)));
    int n = lua_gettop(_lua);    /* number of arguments */
    if (n<1 || lua_type(_lua, 1)!=LUA_TTABLE) return 0;

    osg::Image* image  = lse->getObjectFromTable<osg::Image>(1);
    if (!image)
    {
        OSG_NOTICE<<"Warning: Image:t() can only be called on a Image"<<std::endl;
        return 0;
    }

    lua_pushinteger(_lua, image->t());

    return 1;
}

static int callImageR(lua_State* _lua)
{
    const LuaScriptEngine* lse = reinterpret_cast<const LuaScriptEngine*>(lua_topointer(_lua, lua_upvalueindex(1)));
    int n = lua_gettop(_lua);    /* number of arguments */
    if (n<1 || lua_type(_lua, 1)!=LUA_TTABLE) return 0;

    osg::Image* image  = lse->getObjectFromTable<osg::Image>(1);
    if (!image)
    {
        OSG_NOTICE<<"Warning: Image:r() can only be called on a Image"<<std::endl;
        return 0;
    }

    lua_pushinteger(_lua, image->r());

    return 1;
}

// conversion of a lua value/table to a std::string, supports recursion when tables contain tables
static std::string cpp_tostring(lua_State* _lua, int index)
{
    if (!lua_istable(_lua, index))
    {
        const char* str = lua_tostring(_lua, index);
        if (str)
        {
            return str;
        }
        else
        {
            return "value-cannot-be-converted-to-string";
        }
    }

    // Push another reference to the table on top of the stack (so we know
    // where it is, and this function can work for negative, positive and
    // pseudo indices
    lua_pushvalue(_lua, index);
    // stack now contains: -1 => table
    lua_pushnil(_lua);
    // stack now contains: -1 => nil; -2 => table
    bool first = true;
    std::string str("{");
    while (lua_next(_lua, -2))
    {
        if (!first) str.append(", ");
        else first = false;

        // stack now contains: -1 => value; -2 => key; -3 => table
        // copy the key so that lua_tostring does not modify the original
        lua_pushvalue(_lua, -2);
        // stack now contains: -1 => key; -2 => value; -3 => key; -4 => table

        // handle key
        if (lua_isstring(_lua, -1))
        {
            const char *key = lua_tostring(_lua, -1);
            if (key)
            {
                str.append(key);
                str.append("=");
            }
        }

        // handle value
        if (lua_istable(_lua, -2))
        {
            str.append(cpp_tostring(_lua,-2));
        }
        else if (lua_isfunction(_lua, -2))
        {
            str.append("function");
        }
        else if (lua_isnil(_lua, -2))
        {
            str.append("nil");
        }
        else if (lua_isstring(_lua,-2))
        {
            const char *value = lua_tostring(_lua, -2);
            str.append("\"");
            if (value)
            {
                str.append(value);
            }
            str.append("\"");
        }
        else
        {
            const char *value = lua_tostring(_lua, -2);
            if (value)
            {
                str.append(value);
            }
        }

        // pop value + copy of key, leaving original key
        lua_pop(_lua, 2);
        // stack now contains: -1 => key; -2 => table
    }
    str.append("}");

    // stack now contains: -1 => table (when lua_next returns 0 it pops the key
    // but does not push anything.)
    // Pop table
    lua_pop(_lua, 1);
    // Stack is now the same as it was on entry to this function

    return str;
}

static int tostring(lua_State* _lua)
{
    lua_pushstring(_lua, cpp_tostring(_lua,-1) .c_str());
    return 1;
}

static int callImageGet(lua_State* _lua)
{
    const LuaScriptEngine* lse = reinterpret_cast<const LuaScriptEngine*>(lua_topointer(_lua, lua_upvalueindex(1)));
    int n = lua_gettop(_lua);    /* number of arguments */
    if (n<2 || lua_type(_lua, 1)!=LUA_TTABLE) return 0;

    osg::Image* image  = lse->getObjectFromTable<osg::Image>(1);
    if (!image)
    {
        OSG_NOTICE<<"Warning: Image:get() can only be called on a Image"<<std::endl;
        return 0;
    }

    int image_i = 0;
    int image_j = 0;
    int image_k = 0;
    if (n>=2 && lua_isnumber(_lua, 2)) image_i = static_cast<int>(lua_tonumber(_lua, 2));
    if (n>=3 && lua_isnumber(_lua, 3)) image_j = static_cast<int>(lua_tonumber(_lua, 3));
    if (n>=4 && lua_isnumber(_lua, 4)) image_k = static_cast<int>(lua_tonumber(_lua, 4));

    const unsigned char* ptr = image->data(image_i,image_j,image_k);
    unsigned int numComponents = osg::Image::computeNumComponents(image->getPixelFormat());

    // OSG_NOTICE<<"Need to implement Image::get("<<i<<", "<<j<<", "<<k<<") ptr="<<(void*)ptr<<", numComponents="<<numComponents<<std::endl;

    osg::Vec4d colour;
    switch(image->getDataType())
    {
        case(GL_BYTE): for(unsigned int i=0; i<numComponents; ++i) { colour[i] = static_cast<double>(*(reinterpret_cast<const char*>(ptr)+i)); } break;
        case(GL_UNSIGNED_BYTE): for(unsigned int i=0; i<numComponents; ++i) { colour[i] = static_cast<double>(*(ptr+i)); } break;
        case(GL_SHORT): for(unsigned int i=0; i<numComponents; ++i) { colour[i] = static_cast<double>(*(reinterpret_cast<const short*>(ptr)+i)); } break;
        case(GL_UNSIGNED_SHORT): for(unsigned int i=0; i<numComponents; ++i) { colour[i] = static_cast<double>(*(reinterpret_cast<const unsigned short*>(ptr)+i)); } break;
        case(GL_INT): for(unsigned int i=0; i<numComponents; ++i) { colour[i] = static_cast<double>(*(reinterpret_cast<const int*>(ptr)+i)); } break;
        case(GL_UNSIGNED_INT): for(unsigned int i=0; i<numComponents; ++i) { colour[i] = static_cast<double>(*(reinterpret_cast<const unsigned int*>(ptr)+i)); } break;
        case(GL_FLOAT): for(unsigned int i=0; i<numComponents; ++i) { colour[i] = static_cast<double>(*(reinterpret_cast<const float*>(ptr)+i)); } break;
        case(GL_DOUBLE): for(unsigned int i=0; i<numComponents; ++i) { colour[i] = static_cast<double>(*(reinterpret_cast<const double*>(ptr)+i)); } break;
        default:
            OSG_NOTICE<<"Warning: Unsupported DataType in Image::get()"<<std::endl;
            return 0;
    }

    switch(image->getPixelFormat())
    {
        case(GL_INTENSITY):     lua_pushnumber(_lua, colour[0]); return 1;
        case(GL_LUMINANCE):     lua_pushnumber(_lua, colour[0]); return 1;
        case(GL_ALPHA):         lua_pushnumber(_lua, colour[0]); return 1;
        case(GL_LUMINANCE_ALPHA):
        {
            lua_newtable(_lua); luaL_getmetatable(_lua, "LuaScriptEngine.Table"); lua_setmetatable(_lua, -2);
            lua_pushstring(_lua, "luminance"); lua_pushnumber(_lua, colour[0]); lua_settable(_lua, -3);
            lua_pushstring(_lua, "alpha"); lua_pushnumber(_lua, colour[1]); lua_settable(_lua, -3);
            return 1;
        }
        case(GL_RGB):
        {
            lua_newtable(_lua); luaL_getmetatable(_lua, "LuaScriptEngine.Table"); lua_setmetatable(_lua, -2);
            lua_pushstring(_lua, "red"); lua_pushnumber(_lua, colour[0]); lua_settable(_lua, -3);
            lua_pushstring(_lua, "green"); lua_pushnumber(_lua, colour[1]); lua_settable(_lua, -3);
            lua_pushstring(_lua, "blue"); lua_pushnumber(_lua, colour[2]); lua_settable(_lua, -3);
            return 1;
        }
        case(GL_RGBA):
        {
            lua_newtable(_lua); luaL_getmetatable(_lua, "LuaScriptEngine.Table"); lua_setmetatable(_lua, -2);
            lua_pushstring(_lua, "red"); lua_pushnumber(_lua, colour[0]); lua_settable(_lua, -3);
            lua_pushstring(_lua, "green"); lua_pushnumber(_lua, colour[1]); lua_settable(_lua, -3);
            lua_pushstring(_lua, "blue"); lua_pushnumber(_lua, colour[2]); lua_settable(_lua, -3);
            lua_pushstring(_lua, "alpha"); lua_pushnumber(_lua, colour[3]); lua_settable(_lua, -3);
            return 1;
        }
        case(GL_BGR):
        {
            lua_newtable(_lua); luaL_getmetatable(_lua, "LuaScriptEngine.Table"); lua_setmetatable(_lua, -2);
            lua_pushstring(_lua, "red"); lua_pushnumber(_lua, colour[2]); lua_settable(_lua, -3);
            lua_pushstring(_lua, "green"); lua_pushnumber(_lua, colour[1]); lua_settable(_lua, -3);
            lua_pushstring(_lua, "blue"); lua_pushnumber(_lua, colour[0]); lua_settable(_lua, -3);
            return 1;
        }
        case(GL_BGRA):
        {
            lua_newtable(_lua); luaL_getmetatable(_lua, "LuaScriptEngine.Table"); lua_setmetatable(_lua, -2);
            lua_pushstring(_lua, "red"); lua_pushnumber(_lua, colour[2]); lua_settable(_lua, -3);
            lua_pushstring(_lua, "green"); lua_pushnumber(_lua, colour[1]); lua_settable(_lua, -3);
            lua_pushstring(_lua, "blue"); lua_pushnumber(_lua, colour[0]); lua_settable(_lua, -3);
            lua_pushstring(_lua, "alpha"); lua_pushnumber(_lua, colour[3]); lua_settable(_lua, -3);
            return 1;
        }
    }

    OSG_NOTICE<<"Warning: Image:get() unsupported PixelFormat"<<std::endl;
    return 0;
}

static void setImageColour(osg::Image* image, int image_i, int image_j, int image_k, const osg::Vec4d& colourToWrite)
{
    if (image_i>=image->s() || image_j>=image->t() || image_k>=image->r())
    {
        OSG_NOTICE<<"Warning: Image::set("<<image_i<<", "<<image_j<<", "<<image_k<<") out of range"<<std::endl;
        return;
    }

    unsigned char* ptr = image->data(image_i,image_j,image_k);
    unsigned int numComponents = osg::Image::computeNumComponents(image->getPixelFormat());

    switch(image->getDataType())
    {
        case(GL_BYTE): for(unsigned int i=0; i<numComponents; ++i) { *(reinterpret_cast<char*>(ptr)+i) = static_cast<char>(colourToWrite[i]); } break;
        case(GL_UNSIGNED_BYTE): for(unsigned int i=0; i<numComponents; ++i) { *(reinterpret_cast<unsigned char*>(ptr)+i) = static_cast<unsigned char>(colourToWrite[i]); } break;
        case(GL_SHORT): for(unsigned int i=0; i<numComponents; ++i) { *(reinterpret_cast<short*>(ptr)+i) = static_cast<short>(colourToWrite[i]); } break;
        case(GL_UNSIGNED_SHORT): for(unsigned int i=0; i<numComponents; ++i) { *(reinterpret_cast<unsigned short*>(ptr)+i) = static_cast<unsigned short>(colourToWrite[i]); } break;
        case(GL_INT): for(unsigned int i=0; i<numComponents; ++i) {  *(reinterpret_cast<int*>(ptr)+i) = static_cast<int>(colourToWrite[i]); } break;
        case(GL_UNSIGNED_INT): for(unsigned int i=0; i<numComponents; ++i) { *(reinterpret_cast<unsigned int*>(ptr)+i) = static_cast<unsigned int>(colourToWrite[i]); } break;
        case(GL_FLOAT): for(unsigned int i=0; i<numComponents; ++i) { *(reinterpret_cast<float*>(ptr)+i) = static_cast<float>(colourToWrite[i]); } break;
        case(GL_DOUBLE): for(unsigned int i=0; i<numComponents; ++i) { *(reinterpret_cast<double*>(ptr)+i) = static_cast<double>(colourToWrite[i]); } break;
        default:
            OSG_NOTICE<<"Warning: Unsupported DataType in Image::set()"<<std::endl;
            return;
    }
}

static int callImageSet(lua_State* _lua)
{
    const LuaScriptEngine* lse = reinterpret_cast<const LuaScriptEngine*>(lua_topointer(_lua, lua_upvalueindex(1)));
    int n = lua_gettop(_lua);    /* number of arguments */
    if (n<2 || lua_type(_lua, 1)!=LUA_TTABLE) return 0;

    osg::Image* image  = lse->getObjectFromTable<osg::Image>(1);
    if (!image)
    {
        OSG_NOTICE<<"Warning: Image:set() can only be called on a Image"<<std::endl;
        return 0;
    }

    bool positionSet = false;
    int image_i = 0;
    int image_j = 0;
    int image_k = 0;
    if (n>=3 && lua_isnumber(_lua, 2)) { image_i = static_cast<int>(lua_tonumber(_lua, 2)); positionSet = true; }
    if (n>=4 && lua_isnumber(_lua, 3)) { image_j = static_cast<int>(lua_tonumber(_lua, 3)); positionSet = true; }
    if (n>=5 && lua_isnumber(_lua, 4)) { image_k = static_cast<int>(lua_tonumber(_lua, 4)); positionSet = true; }

    osg::Vec4d colour(1.0,1.0,1.0,1.0);
    if (lua_isnumber(_lua, n))
    {
        colour[0] = colour[1] = colour[2] = colour[3] = lua_tonumber(_lua, n);
    }
    else if (lua_istable(_lua, n))
    {
        lua_getfield(_lua, n, "intensity");
        if (lua_isnumber(_lua, -1)) { double i = lua_tonumber(_lua, -1); colour[0] = i; colour[1] = i; colour[2] = i; colour[3] = i; }
        lua_pop(_lua, 1);

        lua_getfield(_lua, n, "i");
        if (lua_isnumber(_lua, -1)) { double i = lua_tonumber(_lua, -1); colour[0] = i; colour[1] = i; colour[2] = i; colour[3] = i; }
        lua_pop(_lua, 1);


        lua_getfield(_lua, n, "luminance");
        if (lua_isnumber(_lua, -1)) { double l = lua_tonumber(_lua, -1); colour[0] = l; colour[1] = l; colour[2] = l; }
        lua_pop(_lua, 1);

        lua_getfield(_lua, n, "l");
        if (lua_isnumber(_lua, -1)) { double l = lua_tonumber(_lua, -1); colour[0] = l; colour[1] = l; colour[2] = l; }
        lua_pop(_lua, 1);


        lua_getfield(_lua, n, "alpha");
        if (lua_isnumber(_lua, -1)) { double a = lua_tonumber(_lua, -1); colour[3] = a; }
        lua_pop(_lua, 1);

        lua_getfield(_lua, n, "a");
        if (lua_isnumber(_lua, -1)) { double a = lua_tonumber(_lua, -1); colour[3] = a; }
        lua_pop(_lua, 1);


        lua_getfield(_lua, n, "red");
        if (lua_isnumber(_lua, -1)) { double r = lua_tonumber(_lua, -1); colour[0] = r; }
        lua_pop(_lua, 1);

        lua_getfield(_lua, n, "r");
        if (lua_isnumber(_lua, -1)) { double r = lua_tonumber(_lua, -1); colour[0] = r; }
        lua_pop(_lua, 1);


        lua_getfield(_lua, n, "green");
        if (lua_isnumber(_lua, -1)) { double g = lua_tonumber(_lua, -1); colour[1] = g; }
        lua_pop(_lua, 1);

        lua_getfield(_lua, n, "g");
        if (lua_isnumber(_lua, -1)) { double g = lua_tonumber(_lua, -1); colour[1] = g; }
        lua_pop(_lua, 1);


        lua_getfield(_lua, n, "blue");
        if (lua_isnumber(_lua, -1)) { double b = lua_tonumber(_lua, -1); colour[2] = b; }
        lua_pop(_lua, 1);

        lua_getfield(_lua, n, "b");
        if (lua_isnumber(_lua, -1)) { double b = lua_tonumber(_lua, -1); colour[2] = b; }
        lua_pop(_lua, 1);

    }

    // repack the colour data to the final destination form
    osg::Vec4d colourToWrite = colour;
    switch(image->getPixelFormat())
    {
        case(GL_INTENSITY):     colourToWrite[0] = colour[0]; break;
        case(GL_LUMINANCE):     colourToWrite[0] = colour[0]; break;
        case(GL_ALPHA):         colourToWrite[0] = colour[3]; break;
        case(GL_LUMINANCE_ALPHA):
        {
            colourToWrite[0] = colour[0];
            colourToWrite[1] = colour[3];
            break;
        }
        case(GL_RGB):
        case(GL_RGBA):
        {
            // nothing to do as data is already in the correct form
            break;
        }
        case(GL_BGR):
        case(GL_BGRA):
        {
            colourToWrite[0] = colour[2];
            colourToWrite[1] = colour[1];
            colourToWrite[2] = colour[0];
            colourToWrite[3] = colour[3];
            return 1;
        }
    }

    if (positionSet)
    {
        setImageColour(image, image_i,image_j,image_k, colourToWrite);
    }
    else
    {
        for(int k=0; k<image->r(); ++k)
        {
            for(int j=0; j<image->t(); ++j)
            {
                for(int i=0; i<image->s(); ++i)
                {
                    setImageColour(image, i,j,k, colourToWrite);
                }
            }
        }
    }

    return 0;
}

//////////////////////////////////////////////////////////////////////////////////////
//
//  Node Parent
//
static int callGetParent(lua_State* _lua)
{
    const LuaScriptEngine* lse = reinterpret_cast<const LuaScriptEngine*>(lua_topointer(_lua, lua_upvalueindex(1)));
    int n = lua_gettop(_lua);    /* number of arguments */
    if (n<1 || lua_type(_lua, 1)!=LUA_TTABLE) return 0;

    osg::Node* node  = lse->getObjectFromTable<osg::Node>(1);
    if (!node)
    {
        OSG_NOTICE<<"Warning: Node::getParent() can only be called on a Node"<<std::endl;
        return 0;
    }

    int index = 0;
    if (n>=2 && lua_isnumber(_lua, 2))
    {
        index = static_cast<int>(lua_tonumber(_lua, 2));
        if (index>=0 && index<static_cast<int>(node->getNumParents()))
        {
            lse->pushObject(node->getParent(0));
            return 1;
        }
        else
        {
            OSG_NOTICE<<"Warning: Call to node:getParent(index) has an out of range index."<<std::endl;
            return 0;
        }
    }
    else
    {
        OSG_NOTICE<<"Warning: node:getParent() requires a integer parameter."<<std::endl;
        return 0;
    }
}

static int callGetNumParents(lua_State* _lua)
{
    const LuaScriptEngine* lse = reinterpret_cast<const LuaScriptEngine*>(lua_topointer(_lua, lua_upvalueindex(1)));
    int n = lua_gettop(_lua);    /* number of arguments */
    if (n<1 || lua_type(_lua, 1)!=LUA_TTABLE) return 0;

    osg::Node* node  = lse->getObjectFromTable<osg::Node>(1);
    if (!node)
    {
        OSG_NOTICE<<"Warning: Node::getNumParents() can only be called on a Node"<<std::endl;
        return 0;
    }

    lua_pushnumber(_lua, node->getNumParents());
    return 1;
}

//////////////////////////////////////////////////////////////////////////////////////
//
//  Method calling support
//
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

        if (lse->getClassInterface().run(object, compoundClassName, methodName, inputParameters, outputParameters))
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
        osg::ref_ptr<osg::Object> object = osgDB::readRefObjectFile(filename);
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
        osg::ref_ptr<osg::Image> image = osgDB::readRefImageFile(filename);
        if (image.valid())
        {
            lse->pushObject(image.get());
            return 1;
        }
    }
    return 0;
}

static int readShaderFile(lua_State * _lua)
{
    const LuaScriptEngine* lse = reinterpret_cast<const LuaScriptEngine*>(lua_topointer(_lua, lua_upvalueindex(1)));

    int n = lua_gettop(_lua);    /* number of arguments */
    if (n==1 && lua_type(_lua, 1)==LUA_TSTRING)
    {
        std::string filename = lua_tostring(_lua, 1);
        osg::ref_ptr<osg::Shader> shader = osgDB::readRefShaderFile(filename);
        if (shader.valid())
        {
            lse->pushObject(shader.get());
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
        osg::ref_ptr<osg::Node> node = osgDB::readRefNodeFile(filename);
        if (node.valid())
        {
            lse->pushObject(node.get());
            return 1;
        }
    }
    return 0;
}


static int writeFile(lua_State * _lua)
{
    const LuaScriptEngine* lse = reinterpret_cast<const LuaScriptEngine*>(lua_topointer(_lua, lua_upvalueindex(1)));

    int n = lua_gettop(_lua);    /* number of arguments */
    if (n>=2 && lua_type(_lua, 1)==LUA_TTABLE && lua_type(_lua, 2)==LUA_TSTRING)
    {
        osg::Object* object  = lse->getObjectFromTable<osg::Object>(1);
        std::string filename = lua_tostring(_lua, 2);
        if (object)
        {
            osgDB::writeObjectFile(*object, filename);
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

LuaScriptEngine::LuaScriptEngine(const LuaScriptEngine&, const osg::CopyOp&):
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
        lua_setglobal(_lua, "readFile");
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

    // provide global new method for read Images
    {
        lua_pushlightuserdata(_lua, this);
        lua_pushcclosure(_lua, readShaderFile, 1);
        lua_setglobal(_lua, "readShaderFile");
    }

    // provide global new method for read Images
    {
        lua_pushlightuserdata(_lua, this);
        lua_pushcclosure(_lua, writeFile, 1);
        lua_setglobal(_lua, "writeFile");
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

        lua_pushstring(_lua, "__tostring");
        lua_pushlightuserdata(_lua, this);
        lua_pushcclosure(_lua, tostring, 1);
        lua_settable(_lua, -3);

        lua_pop(_lua,1);
    }

    // Set up the __tostring methods to be able to convert tables into strings so they can be output for debugging purposes.
    {
        luaL_newmetatable(_lua, "LuaScriptEngine.Table");

        lua_pushstring(_lua, "__tostring");
        lua_pushlightuserdata(_lua, this);
        lua_pushcclosure(_lua, tostring, 1);
        lua_settable(_lua, -3);

        lua_pop(_lua,1);
    }

    // Set up the __newindex and __index methods for looking up implementations of Object properties
    {
        luaL_newmetatable(_lua, "LuaScriptEngine.Container");

        lua_pushstring(_lua, "__index");
        lua_pushlightuserdata(_lua, this);
        lua_pushcclosure(_lua, getContainerProperty, 1);
        lua_settable(_lua, -3);

        lua_pushstring(_lua, "__newindex");
        lua_pushlightuserdata(_lua, this);
        lua_pushcclosure(_lua, setContainerProperty, 1);
        lua_settable(_lua, -3);

        lua_pop(_lua,1);
    }

    // Set up the __newindex and __index methods for looking up implementations of Object properties
    {
        luaL_newmetatable(_lua, "LuaScriptEngine.Map");

        lua_pushstring(_lua, "__index");
        lua_pushlightuserdata(_lua, this);
        lua_pushcclosure(_lua, getMapProperty, 1);
        lua_settable(_lua, -3);

        lua_pushstring(_lua, "__newindex");
        lua_pushlightuserdata(_lua, this);
        lua_pushcclosure(_lua, setMapProperty, 1);
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

    const LuaScriptEngine* _lse;
    lua_State* _lua;

    PushStackValueVisitor(const LuaScriptEngine* lse) : _lse(lse) { _lua = const_cast<LuaScriptEngine*>(lse)->getLuaState(); }

    virtual void apply(bool value)                      { lua_pushboolean(_lua, value ? 1 : 0); }
    virtual void apply(char value)                      { lua_pushnumber(_lua, value); }
    virtual void apply(unsigned char value)             { lua_pushnumber(_lua, value); }
    virtual void apply(short value)                     { lua_pushnumber(_lua, value); }
    virtual void apply(unsigned short value)            { lua_pushnumber(_lua, value); }
    virtual void apply(int value)                       { lua_pushnumber(_lua, value); }
    virtual void apply(unsigned int value)              { lua_pushnumber(_lua, value); }
    virtual void apply(float value)                     { lua_pushnumber(_lua, value); }
    virtual void apply(double value)                    { lua_pushnumber(_lua, value); }
    virtual void apply(const std::string& value)        { lua_pushlstring(_lua, &value[0], value.size()); }

    virtual void apply(const osg::Vec2b& value)         { _lse->pushValue(value); }
    virtual void apply(const osg::Vec3b& value)         { _lse->pushValue(value); }
    virtual void apply(const osg::Vec4b& value)         { _lse->pushValue(value); }

    virtual void apply(const osg::Vec2ub& value)         { _lse->pushValue(value); }
    virtual void apply(const osg::Vec3ub& value)         { _lse->pushValue(value); }
    virtual void apply(const osg::Vec4ub& value)         { _lse->pushValue(value); }

    virtual void apply(const osg::Vec2s& value)         { _lse->pushValue(value); }
    virtual void apply(const osg::Vec3s& value)         { _lse->pushValue(value); }
    virtual void apply(const osg::Vec4s& value)         { _lse->pushValue(value); }

    virtual void apply(const osg::Vec2us& value)         { _lse->pushValue(value); }
    virtual void apply(const osg::Vec3us& value)         { _lse->pushValue(value); }
    virtual void apply(const osg::Vec4us& value)         { _lse->pushValue(value); }

    virtual void apply(const osg::Vec2i& value)         { _lse->pushValue(value); }
    virtual void apply(const osg::Vec3i& value)         { _lse->pushValue(value); }
    virtual void apply(const osg::Vec4i& value)         { _lse->pushValue(value); }

    virtual void apply(const osg::Vec2ui& value)         { _lse->pushValue(value); }
    virtual void apply(const osg::Vec3ui& value)         { _lse->pushValue(value); }
    virtual void apply(const osg::Vec4ui& value)         { _lse->pushValue(value); }

    virtual void apply(const osg::Vec2f& value)         { _lse->pushValue(value); }
    virtual void apply(const osg::Vec3f& value)         { _lse->pushValue(value); }
    virtual void apply(const osg::Vec4f& value)         { _lse->pushValue(value); }

    virtual void apply(const osg::Vec2d& value)         { _lse->pushValue(value); }
    virtual void apply(const osg::Vec3d& value)         { _lse->pushValue(value); }
    virtual void apply(const osg::Vec4d& value)         { _lse->pushValue(value); }

    virtual void apply(const osg::Quat& value)          { _lse->pushValue(value); }
    virtual void apply(const osg::Plane& value)         { _lse->pushValue(value); }
    virtual void apply(const osg::Matrixf& value)       { _lse->pushValue(value); }
    virtual void apply(const osg::Matrixd& value)       { _lse->pushValue(value); }
};

#if LUA_VERSION_NUM<=501
    #define lua_rawlen lua_strlen
#endif

class GetStackValueVisitor : public osg::ValueObject::SetValueVisitor
{
public:

    const LuaScriptEngine* _lse;
    lua_State* _lua;
    int _index;
    int _numberToPop;
    bool _success;

    GetStackValueVisitor(const LuaScriptEngine* lse, int index) : _lse(lse), _lua(0), _index(index), _numberToPop(0), _success(false) { _lua = const_cast<LuaScriptEngine*>(lse  )->getLuaState(); }


    virtual void apply(bool& value)             { if (lua_isboolean(_lua, _index)) { value = (lua_toboolean(_lua, _index)!=0); _success=true; _numberToPop = 1; } }
    virtual void apply(char& value)             { if (lua_isnumber(_lua, _index)) { value = lua_tonumber(_lua, _index)!=0; _success=true; _numberToPop = 1; } }
    virtual void apply(unsigned char& value)    { if (lua_isnumber(_lua, _index)) { value = lua_tonumber(_lua, _index)!=0; _success=true; _numberToPop = 1; } }
    virtual void apply(short& value)            { if (lua_isnumber(_lua, _index)) { value = lua_tonumber(_lua, _index)!=0; _success=true; _numberToPop = 1; } }
    virtual void apply(unsigned short& value)   { if (lua_isnumber(_lua, _index)) { value = lua_tonumber(_lua, _index)!=0; _success=true; _numberToPop = 1; } }
    virtual void apply(int& value)              { if (lua_isnumber(_lua, _index)) { value = lua_tonumber(_lua, _index)!=0; _success=true; _numberToPop = 1; } }
    virtual void apply(unsigned int& value)     { if (lua_isnumber(_lua, _index)) { value = lua_tonumber(_lua, _index)!=0; _success=true; _numberToPop = 1; } }
    virtual void apply(float& value)            { if (lua_isnumber(_lua, _index)) { value = lua_tonumber(_lua, _index)!=0; _success=true; _numberToPop = 1; } }
    virtual void apply(double& value)           { if (lua_isnumber(_lua, _index)) { value = lua_tonumber(_lua, _index)!=0; _success=true; _numberToPop = 1; } }
    virtual void apply(std::string& value)      { if (lua_isstring(_lua, _index)) { value = std::string(lua_tostring(_lua, _index), lua_rawlen(_lua, _index)); _numberToPop = 1; } }
    virtual void apply(osg::Vec2f& value)       { if (_lse->getValue(_index, value)) { _success=true; _numberToPop = 2;} }
    virtual void apply(osg::Vec3f& value)       { if (_lse->getValue(_index, value)) { _success=true; _numberToPop = 2; } }
    virtual void apply(osg::Vec4f& value)       { if (_lse->getValue(_index, value)) { _success=true; _numberToPop = 4; } }
    virtual void apply(osg::Vec2d& value)       { if (_lse->getValue(_index, value)) { _success=true; _numberToPop = 2; } }
    virtual void apply(osg::Vec3d& value)       { if (_lse->getValue(_index, value)) { _success=true; _numberToPop = 3; } }
    virtual void apply(osg::Vec4d& value)       { if (_lse->getValue(_index, value)) { _success=true; _numberToPop = 4; } }
    virtual void apply(osg::Quat& value)        { if (_lse->getValue(_index, value)) { _success=true; _numberToPop = 4; } }
    virtual void apply(osg::Plane& value)       { if (_lse->getValue(_index, value)) { _success=true; _numberToPop = 4; } }
    virtual void apply(osg::Matrixf& value)     { if (_lse->getValue(_index, value)) { _success = true; _numberToPop = 16; } }
    virtual void apply(osg::Matrixd& value)     { if (_lse->getValue(_index, value)) { _success = true; _numberToPop = 16; } }
    virtual void apply(osg::BoundingBoxf& value) { if (_lse->getValue(_index, value)) { _success=true; } }
    virtual void apply(osg::BoundingBoxd& value) { if (_lse->getValue(_index, value)) { _success=true; } }
    virtual void apply(osg::BoundingSpheref& value) { if (_lse->getValue(_index, value)) { _success=true; } }
    virtual void apply(osg::BoundingSphered& value) { if (_lse->getValue(_index, value)) { _success=true; } }
};

int LuaScriptEngine::pushPropertyToStack(osg::Object* object, const std::string& propertyName) const
{
    osgDB::BaseSerializer::Type type;
    if (!_ci.getPropertyType(object, propertyName, type))
    {
        if (_ci.hasMethod(object, propertyName))
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

        OSG_INFO<<"LuaScriptEngine::pushPropertyToStack("<<object<<", "<<propertyName<<") no property found."<<std::endl;
        return 0;
    }

    switch(type)
    {
        case(osgDB::BaseSerializer::RW_BOOL):
        {
            bool value;
            if (_ci.getProperty(object, propertyName, value))
            {
                lua_pushboolean(_lua, value ? 1 : 0);
                return 1;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_STRING):
        {
            std::string value;
            if (_ci.getProperty(object, propertyName, value))
            {
                lua_pushstring(_lua, value.c_str());
                return 1;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_GLENUM):
        {
            GLenum value;
            if (_ci.getProperty(object, propertyName, value))
            {
                std::string enumString = lookUpGLenumString(value);
                lua_pushstring(_lua, enumString.c_str());
                return 1;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_ENUM):
        {
            int value;
            if (_ci.getProperty(object, propertyName, value))
            {
                osgDB::BaseSerializer* serializer = _ci.getSerializer(object, propertyName, type);
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
        case(osgDB::BaseSerializer::RW_SHORT):
        {
            short value;
            if (_ci.getProperty(object, propertyName, value))
            {
                lua_pushinteger(_lua, value);
                return 1;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_USHORT):
        {
            unsigned short value;
            if (_ci.getProperty(object, propertyName, value))
            {
                lua_pushinteger(_lua, value);
                return 1;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_INT):
        {
            int value;
            if (_ci.getProperty(object, propertyName, value))
            {
                lua_pushinteger(_lua, value);
                return 1;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_UINT):
        {
            unsigned int value;
            if (_ci.getProperty(object, propertyName, value))
            {
                lua_pushinteger(_lua, value);
                return 1;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_FLOAT):
        {
            float value;
            if (_ci.getProperty(object, propertyName, value))
            {
                lua_pushnumber(_lua, value);
                return 1;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_DOUBLE):
        {
            double value;
            if (_ci.getProperty(object, propertyName, value))
            {
                lua_pushnumber(_lua, value);
                return 1;
            }
            break;
        }

        case(osgDB::BaseSerializer::RW_VEC2B): if (getPropertyAndPushValue<osg::Vec2b>(object, propertyName)) return 1; break;
        case(osgDB::BaseSerializer::RW_VEC3B): if (getPropertyAndPushValue<osg::Vec3b>(object, propertyName)) return 1; break;
        case(osgDB::BaseSerializer::RW_VEC4B): if (getPropertyAndPushValue<osg::Vec4b>(object, propertyName)) return 1; break;

        case(osgDB::BaseSerializer::RW_VEC2UB): if (getPropertyAndPushValue<osg::Vec2ub>(object, propertyName)) return 1; break;
        case(osgDB::BaseSerializer::RW_VEC3UB): if (getPropertyAndPushValue<osg::Vec3ub>(object, propertyName)) return 1; break;
        case(osgDB::BaseSerializer::RW_VEC4UB): if (getPropertyAndPushValue<osg::Vec4ub>(object, propertyName)) return 1; break;

        case(osgDB::BaseSerializer::RW_VEC2S): if (getPropertyAndPushValue<osg::Vec2s>(object, propertyName)) return 1; break;
        case(osgDB::BaseSerializer::RW_VEC3S): if (getPropertyAndPushValue<osg::Vec3s>(object, propertyName)) return 1; break;
        case(osgDB::BaseSerializer::RW_VEC4S): if (getPropertyAndPushValue<osg::Vec4s>(object, propertyName)) return 1; break;

        case(osgDB::BaseSerializer::RW_VEC2US): if (getPropertyAndPushValue<osg::Vec2us>(object, propertyName)) return 1; break;
        case(osgDB::BaseSerializer::RW_VEC3US): if (getPropertyAndPushValue<osg::Vec3us>(object, propertyName)) return 1; break;
        case(osgDB::BaseSerializer::RW_VEC4US): if (getPropertyAndPushValue<osg::Vec4us>(object, propertyName)) return 1; break;

        case(osgDB::BaseSerializer::RW_VEC2I): if (getPropertyAndPushValue<osg::Vec2i>(object, propertyName)) return 1; break;
        case(osgDB::BaseSerializer::RW_VEC3I): if (getPropertyAndPushValue<osg::Vec3i>(object, propertyName)) return 1; break;
        case(osgDB::BaseSerializer::RW_VEC4I): if (getPropertyAndPushValue<osg::Vec4i>(object, propertyName)) return 1; break;

        case(osgDB::BaseSerializer::RW_VEC2UI): if (getPropertyAndPushValue<osg::Vec2ui>(object, propertyName)) return 1; break;
        case(osgDB::BaseSerializer::RW_VEC3UI): if (getPropertyAndPushValue<osg::Vec3ui>(object, propertyName)) return 1; break;
        case(osgDB::BaseSerializer::RW_VEC4UI): if (getPropertyAndPushValue<osg::Vec4ui>(object, propertyName)) return 1; break;

        case(osgDB::BaseSerializer::RW_VEC2F): if (getPropertyAndPushValue<osg::Vec2f>(object, propertyName)) return 1; break;
        case(osgDB::BaseSerializer::RW_VEC3F): if (getPropertyAndPushValue<osg::Vec3f>(object, propertyName)) return 1; break;
        case(osgDB::BaseSerializer::RW_VEC4F): if (getPropertyAndPushValue<osg::Vec4f>(object, propertyName)) return 1; break;

        case(osgDB::BaseSerializer::RW_VEC2D): if (getPropertyAndPushValue<osg::Vec2d>(object, propertyName)) return 1; break;
        case(osgDB::BaseSerializer::RW_VEC3D): if (getPropertyAndPushValue<osg::Vec3d>(object, propertyName)) return 1; break;
        case(osgDB::BaseSerializer::RW_VEC4D): if (getPropertyAndPushValue<osg::Vec4d>(object, propertyName)) return 1; break;

        case(osgDB::BaseSerializer::RW_QUAT): if (getPropertyAndPushValue<osg::Quat>(object, propertyName)) return 1; break;
        case(osgDB::BaseSerializer::RW_PLANE): if (getPropertyAndPushValue<osg::Plane>(object, propertyName)) return 1; break;

        #ifdef OSG_USE_FLOAT_MATRIX
        case(osgDB::BaseSerializer::RW_MATRIX):
#endif
        case(osgDB::BaseSerializer::RW_MATRIXF):
        {
            osg::Matrixf value;
            if (_ci.getProperty(object, propertyName, value))
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
            if (_ci.getProperty(object, propertyName, value))
            {
                pushValue(value);
                return 1;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_BOUNDINGBOXF):
        {
            osg::BoundingBoxf value;
            if (_ci.getProperty(object, propertyName, value))
            {
                pushValue(value);
                return 1;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_BOUNDINGBOXD):
        {
            osg::BoundingBoxd value;
            if (_ci.getProperty(object, propertyName, value))
            {
                pushValue(value);
                return 1;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_BOUNDINGSPHEREF):
        {
            osg::BoundingSpheref value;
            if (_ci.getProperty(object, propertyName, value))
            {
                pushValue(value);
                return 1;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_BOUNDINGSPHERED):
        {
            osg::BoundingSphered value;
            if (_ci.getProperty(object, propertyName, value))
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
            if (_ci.getProperty(object, propertyName, value))
            {
                pushObject(value);
                return 1;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_VECTOR):
        case(osgDB::BaseSerializer::RW_MAP):
        {
            pushContainer(object, propertyName);
            return 1;
        }
        default:
            break;
    }

    OSG_NOTICE<<"LuaScriptEngine::pushPropertyToStack("<<object<<", "<<propertyName<<") property of type = "<<_ci.getTypeName(type)<<" error, not supported."<<std::endl;
    return 0;
}

std::string LuaScriptEngine::lookUpGLenumString(GLenum value) const
{
    osgDB::ObjectWrapperManager* ow = osgDB::Registry::instance()->getObjectWrapperManager();

    {
        const osgDB::IntLookup& lookup = ow->getLookupMap()["GL"];
        const osgDB::IntLookup::ValueToString& vts = lookup.getValueToString();
        osgDB::IntLookup::ValueToString::const_iterator itr = vts.find(value);
        if (itr!=vts.end()) return itr->second;
    }

    {
        const osgDB::IntLookup& lookup = ow->getLookupMap()["PrimitiveType"];
        const osgDB::IntLookup::ValueToString& vts = lookup.getValueToString();
        osgDB::IntLookup::ValueToString::const_iterator itr = vts.find(value);
        if (itr!=vts.end()) return itr->second;
    }

    OSG_NOTICE<<"Warning: LuaScriptEngine did not find valid GL enum value for GLenum value: "<<value<<std::endl;

    return std::string();
}

GLenum LuaScriptEngine::lookUpGLenumValue(const std::string& str) const
{
    osgDB::ObjectWrapperManager* ow = osgDB::Registry::instance()->getObjectWrapperManager();

    {
        const osgDB::IntLookup& lookup = ow->getLookupMap()["GL"];
        const osgDB::IntLookup::StringToValue& stv = lookup.getStringToValue();
        osgDB::IntLookup::StringToValue::const_iterator itr = stv.find(str);
        if (itr!=stv.end()) return itr->second;
    }

    {
        const osgDB::IntLookup& lookup = ow->getLookupMap()["PrimitiveType"];
        const osgDB::IntLookup::StringToValue& stv = lookup.getStringToValue();
        osgDB::IntLookup::StringToValue::const_iterator itr = stv.find(str);
        if (itr!=stv.end()) return itr->second;
    }

    OSG_NOTICE<<"Warning: LuaScriptEngine did not find valid GL enum value for string value: "<<str<<std::endl;

    return GL_NONE;
}


int LuaScriptEngine::pushDataToStack(SerializerScratchPad* ssp) const
{
    switch(ssp->dataType)
    {
        case(osgDB::BaseSerializer::RW_BOOL):
        {
            bool value;
            if (ssp->get(value))
            {
                lua_pushboolean(_lua, value ? 1 : 0);
                return 1;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_STRING):
        {
            std::string value;
            if (ssp->get(value))
            {
                lua_pushstring(_lua, value.c_str());
                return 1;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_GLENUM):
        {
            GLenum value;
            if (ssp->get(value))
            {
                std::string enumString = lookUpGLenumString(value);
                lua_pushstring(_lua, enumString.c_str());
                return 1;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_ENUM):
        {
            int value;
            if (ssp->get(value))
            {
                lua_pushinteger(_lua, value);
                return 1;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_INT):
        {
            int value;
            if (ssp->get(value))
            {
                lua_pushinteger(_lua, value);
                return 1;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_UINT):
        {
            unsigned int value;
            if (ssp->get(value))
            {
                lua_pushinteger(_lua, value);
                return 1;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_SHORT):
        {
            short value;
            if (ssp->get(value))
            {
                lua_pushinteger(_lua, value);
                return 1;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_USHORT):
        {
            unsigned short value;
            if (ssp->get(value))
            {
                lua_pushinteger(_lua, value);
                return 1;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_FLOAT):
        {
            float value;
            if (ssp->get(value))
            {
                lua_pushnumber(_lua, value);
                return 1;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_DOUBLE):
        {
            double value;
            if (ssp->get(value))
            {
                lua_pushnumber(_lua, value);
                return 1;
            }
            break;
        }

        case(osgDB::BaseSerializer::RW_VEC2B): if (pushValueToStack<osg::Vec2b>(ssp)) return 1; break;
        case(osgDB::BaseSerializer::RW_VEC3B): if (pushValueToStack<osg::Vec3b>(ssp)) return 1; break;
        case(osgDB::BaseSerializer::RW_VEC4B): if (pushValueToStack<osg::Vec4b>(ssp)) return 1; break;

        case(osgDB::BaseSerializer::RW_VEC2UB): if (pushValueToStack<osg::Vec2ub>(ssp)) return 1; break;
        case(osgDB::BaseSerializer::RW_VEC3UB): if (pushValueToStack<osg::Vec3ub>(ssp)) return 1; break;
        case(osgDB::BaseSerializer::RW_VEC4UB): if (pushValueToStack<osg::Vec4ub>(ssp)) return 1; break;

        case(osgDB::BaseSerializer::RW_VEC2S): if (pushValueToStack<osg::Vec2s>(ssp)) return 1; break;
        case(osgDB::BaseSerializer::RW_VEC3S): if (pushValueToStack<osg::Vec3s>(ssp)) return 1; break;
        case(osgDB::BaseSerializer::RW_VEC4S): if (pushValueToStack<osg::Vec4s>(ssp)) return 1; break;

        case(osgDB::BaseSerializer::RW_VEC2US): if (pushValueToStack<osg::Vec2us>(ssp)) return 1; break;
        case(osgDB::BaseSerializer::RW_VEC3US): if (pushValueToStack<osg::Vec3us>(ssp)) return 1; break;
        case(osgDB::BaseSerializer::RW_VEC4US): if (pushValueToStack<osg::Vec4us>(ssp)) return 1; break;

        case(osgDB::BaseSerializer::RW_VEC2I): if (pushValueToStack<osg::Vec2i>(ssp)) return 1; break;
        case(osgDB::BaseSerializer::RW_VEC3I): if (pushValueToStack<osg::Vec3i>(ssp)) return 1; break;
        case(osgDB::BaseSerializer::RW_VEC4I): if (pushValueToStack<osg::Vec4i>(ssp)) return 1; break;

        case(osgDB::BaseSerializer::RW_VEC2UI): if (pushValueToStack<osg::Vec2ui>(ssp)) return 1; break;
        case(osgDB::BaseSerializer::RW_VEC3UI): if (pushValueToStack<osg::Vec3ui>(ssp)) return 1; break;
        case(osgDB::BaseSerializer::RW_VEC4UI): if (pushValueToStack<osg::Vec4ui>(ssp)) return 1; break;

        case(osgDB::BaseSerializer::RW_VEC2F): if (pushValueToStack<osg::Vec2f>(ssp)) return 1; break;
        case(osgDB::BaseSerializer::RW_VEC3F): if (pushValueToStack<osg::Vec3f>(ssp)) return 1; break;
        case(osgDB::BaseSerializer::RW_VEC4F): if (pushValueToStack<osg::Vec4f>(ssp)) return 1; break;

        case(osgDB::BaseSerializer::RW_VEC2D): if (pushValueToStack<osg::Vec2d>(ssp)) return 1; break;
        case(osgDB::BaseSerializer::RW_VEC3D): if (pushValueToStack<osg::Vec3d>(ssp)) return 1; break;
        case(osgDB::BaseSerializer::RW_VEC4D): if (pushValueToStack<osg::Vec4d>(ssp)) return 1; break;

        case(osgDB::BaseSerializer::RW_QUAT): if (pushValueToStack<osg::Quat>(ssp)) return 1; break;
        case(osgDB::BaseSerializer::RW_PLANE): if (pushValueToStack<osg::Plane>(ssp)) return 1; break;

#ifdef OSG_USE_FLOAT_MATRIX
        case(osgDB::BaseSerializer::RW_MATRIX):
#endif
        case(osgDB::BaseSerializer::RW_MATRIXF):
        {
            osg::Matrixf value;
            if (ssp->get(value))
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
            if (ssp->get(value))
            {
                pushValue(value);
                return 1;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_BOUNDINGBOXF):
        {
            osg::BoundingBoxf value;
            if (ssp->get(value))
            {
                pushValue(value);
                return 1;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_BOUNDINGBOXD):
        {
            osg::BoundingBoxd value;
            if (ssp->get(value))
            {
                pushValue(value);
                return 1;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_BOUNDINGSPHEREF):
        {
            osg::BoundingSpheref value;
            if (ssp->get(value))
            {
                pushValue(value);
                return 1;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_BOUNDINGSPHERED):
        {
            osg::BoundingSphered value;
            if (ssp->get(value))
            {
                pushValue(value);
                return 1;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_IMAGE):
        case(osgDB::BaseSerializer::RW_OBJECT):
        {
            osg::Object* value = 0;
            if (ssp->get(value))
            {
                pushObject(value);
                return 1;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_LIST):
        {
            break;
        }
        case(osgDB::BaseSerializer::RW_VECTOR):
        {
            break;
        }
        default:
            break;
    }

    OSG_NOTICE<<"LuaScriptEngine::pushDataToStack() property of type = "<<_ci.getTypeName(ssp->dataType)<<" error, not supported."<<std::endl;
    return 0;
}

int LuaScriptEngine::getDataFromStack(SerializerScratchPad* ssp, osgDB::BaseSerializer::Type type, int pos) const
{
    pos = getAbsolutePos(pos);

    if (type==osgDB::BaseSerializer::RW_UNDEFINED) type = LuaScriptEngine::getType(pos);

    switch(type)
    {
        case(osgDB::BaseSerializer::RW_BOOL):
        {
            if (lua_isboolean(_lua, pos))
            {
                ssp->set(static_cast<bool>(lua_toboolean(_lua, pos)!=0));
                return 0;
            }
            else if (lua_isnumber(_lua, pos))
            {
                ssp->set(static_cast<bool>(lua_tonumber(_lua, pos)!=0));
                return 0;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_STRING):
        {
            if (lua_isstring(_lua, pos))
            {
                ssp->set(std::string(lua_tostring(_lua, pos)));
                return 0;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_GLENUM):
        {
            if (lua_isnumber(_lua, pos))
            {
                ssp->set(static_cast<GLenum>(lua_tonumber(_lua, pos)));
                return 0;
            }
            else if (lua_isstring(_lua, pos))
            {
                const char* enumString = lua_tostring(_lua, pos);
                GLenum value = lookUpGLenumValue(enumString); //getValue("GL",enumString);

                ssp->set(value);
                return 0;
            }
            OSG_NOTICE<<"LuaScriptEngine::getDataFromStack() osgDB::BaseSerializer::RW_GLENUM Failed"<<std::endl;
            break;
        }
        case(osgDB::BaseSerializer::RW_ENUM):
        {
            if (lua_isnumber(_lua, pos))
            {
                ssp->set(static_cast<int>(lua_tonumber(_lua, pos)));
                return 0;
            }
            else if (lua_isstring(_lua, pos))
            {
                OSG_NOTICE<<"LuaScriptEngine::getDataFromStack() osgDB::BaseSerializer::RW_ENUM Failed to convert string"<<std::endl;
                return 0;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_SHORT):
        {
            if (lua_isnumber(_lua, pos))
            {
                ssp->set(static_cast<short>(lua_tonumber(_lua, pos)));
                return 0;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_USHORT):
        {
            if (lua_isnumber(_lua, pos))
            {
                ssp->set(static_cast<unsigned short>(lua_tonumber(_lua, pos)));
                return 0;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_INT):
        {
            if (lua_isnumber(_lua, pos))
            {
                ssp->set(static_cast<int>(lua_tonumber(_lua, pos)));
                return 0;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_UINT):
        {
            if (lua_isnumber(_lua, pos))
            {
                ssp->set(static_cast<unsigned int>(lua_tonumber(_lua, pos)));
                return 0;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_FLOAT):
        {
            if (lua_isnumber(_lua, pos))
            {
                ssp->set(static_cast<float>(lua_tonumber(_lua, pos)));
                return 0;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_DOUBLE):
        {
            if (lua_isnumber(_lua, pos))
            {
                ssp->set(static_cast<double>(lua_tonumber(_lua, pos)));
                return 0;
            }
            break;
        }

        case(osgDB::BaseSerializer::RW_VEC2B): if (getDataFromStack<osg::Vec2b>(ssp, pos)) return 0; break;
        case(osgDB::BaseSerializer::RW_VEC3B): if (getDataFromStack<osg::Vec3b>(ssp, pos)) return 0; break;
        case(osgDB::BaseSerializer::RW_VEC4B): if (getDataFromStack<osg::Vec4b>(ssp, pos)) return 0; break;

        case(osgDB::BaseSerializer::RW_VEC2UB): if (getDataFromStack<osg::Vec2ub>(ssp, pos)) return 0; break;
        case(osgDB::BaseSerializer::RW_VEC3UB): if (getDataFromStack<osg::Vec3ub>(ssp, pos)) return 0; break;
        case(osgDB::BaseSerializer::RW_VEC4UB): if (getDataFromStack<osg::Vec4ub>(ssp, pos)) return 0; break;

        case(osgDB::BaseSerializer::RW_VEC2S): if (getDataFromStack<osg::Vec2s>(ssp, pos)) return 0; break;
        case(osgDB::BaseSerializer::RW_VEC3S): if (getDataFromStack<osg::Vec3s>(ssp, pos)) return 0; break;
        case(osgDB::BaseSerializer::RW_VEC4S): if (getDataFromStack<osg::Vec4s>(ssp, pos)) return 0; break;

        case(osgDB::BaseSerializer::RW_VEC2US): if (getDataFromStack<osg::Vec2us>(ssp, pos)) return 0; break;
        case(osgDB::BaseSerializer::RW_VEC3US): if (getDataFromStack<osg::Vec3us>(ssp, pos)) return 0; break;
        case(osgDB::BaseSerializer::RW_VEC4US): if (getDataFromStack<osg::Vec4us>(ssp, pos)) return 0; break;

        case(osgDB::BaseSerializer::RW_VEC2I): if (getDataFromStack<osg::Vec2i>(ssp, pos)) return 0; break;
        case(osgDB::BaseSerializer::RW_VEC3I): if (getDataFromStack<osg::Vec3i>(ssp, pos)) return 0; break;
        case(osgDB::BaseSerializer::RW_VEC4I): if (getDataFromStack<osg::Vec4i>(ssp, pos)) return 0; break;

        case(osgDB::BaseSerializer::RW_VEC2UI): if (getDataFromStack<osg::Vec2ui>(ssp, pos)) return 0; break;
        case(osgDB::BaseSerializer::RW_VEC3UI): if (getDataFromStack<osg::Vec3ui>(ssp, pos)) return 0; break;
        case(osgDB::BaseSerializer::RW_VEC4UI): if (getDataFromStack<osg::Vec4ui>(ssp, pos)) return 0; break;

        case(osgDB::BaseSerializer::RW_VEC2F): if (getDataFromStack<osg::Vec2f>(ssp, pos)) return 0; break;
        case(osgDB::BaseSerializer::RW_VEC3F): if (getDataFromStack<osg::Vec3f>(ssp, pos)) return 0; break;
        case(osgDB::BaseSerializer::RW_VEC4F): if (getDataFromStack<osg::Vec4f>(ssp, pos)) return 0; break;

        case(osgDB::BaseSerializer::RW_VEC2D): if (getDataFromStack<osg::Vec2d>(ssp, pos)) return 0; break;
        case(osgDB::BaseSerializer::RW_VEC3D): if (getDataFromStack<osg::Vec3d>(ssp, pos)) return 0; break;
        case(osgDB::BaseSerializer::RW_VEC4D): if (getDataFromStack<osg::Vec4d>(ssp, pos)) return 0; break;

        case(osgDB::BaseSerializer::RW_QUAT): if (getDataFromStack<osg::Quat>(ssp, pos)) return 0; break;
        case(osgDB::BaseSerializer::RW_PLANE): if (getDataFromStack<osg::Plane>(ssp, pos)) return 0; break;

        #ifdef OSG_USE_FLOAT_MATRIX
        case(osgDB::BaseSerializer::RW_MATRIX):
#endif
        case(osgDB::BaseSerializer::RW_MATRIXF):
        {
            osg::Matrixd value;
            if (getValue(pos, value))
            {
                ssp->set(value);
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
            if (getValue(pos, value))
            {
                ssp->set(value);
                return 0;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_BOUNDINGBOXF):
        {
            osg::BoundingBoxf value;
            if (getValue(pos, value))
            {
                ssp->set(value);
                return 0;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_BOUNDINGBOXD):
        {
            osg::BoundingBoxd value;
            if (getValue(pos, value))
            {
                ssp->set(value);
                return 0;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_BOUNDINGSPHEREF):
        {
            osg::BoundingSpheref value;
            if (getValue(pos, value))
            {
                ssp->set(value);
                return 0;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_BOUNDINGSPHERED):
        {
            osg::BoundingSphered value;
            if (getValue(pos, value))
            {
                ssp->set(value);
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
            if (lua_istable(_lua, pos))
            {
                osg::Object* value = 0;
                lua_pushstring(_lua, "object_ptr");
                lua_rawget(_lua, pos);
                if (lua_type(_lua, -1)==LUA_TUSERDATA) value = *const_cast<osg::Object**>(reinterpret_cast<const osg::Object**>(lua_touserdata(_lua,-1)));
                lua_pop(_lua, 1);

                if (value)
                {
                    ssp->set(value);
                    return 0;
                }
                else
                {
                    OSG_NOTICE<<"Error: lua type '"<<lua_typename(_lua,lua_type(_lua, pos))<<"' cannot be assigned." <<std::endl;
                }
            }
            else if (lua_isnil(_lua, pos))
            {
                OSG_NOTICE<<"Assigning property object (nil) to to object"<<std::endl;
                osg::Object* value = 0;
                ssp->set(value);
                return 0;
            }
            else
            {
                OSG_NOTICE<<"Error: lua type '"<<lua_typename(_lua,lua_type(_lua, pos))<<"' cannot be assigned."<<std::endl;
                return 0;
            }
            break;
        }
        default:
            break;
    }
    OSG_NOTICE<<"LuaScriptEngine::getDataFromStack() property of type = "<<_ci.getTypeName(type)<<" not matched"<<std::endl;
    return 0;

}


int LuaScriptEngine::setPropertyFromStack(osg::Object* object, const std::string& propertyName) const
{
    osgDB::BaseSerializer::Type type;
    if (!_ci.getPropertyType(object, propertyName, type))
    {
        if (lua_type(_lua,-1)==LUA_TFUNCTION)
        {
            int ref = luaL_ref(_lua, LUA_REGISTRYINDEX);
            osg::ref_ptr<LuaCallbackObject> lco = new LuaCallbackObject(propertyName, this, ref);

            osg::UserDataContainer* udc = object->getOrCreateUserDataContainer();
            unsigned int objectIndex = udc->getUserObjectIndex(propertyName);
            if (objectIndex < udc->getNumUserObjects())
            {
                udc->setUserObject(objectIndex, lco.get());
            }
            else
            {
                udc->addUserObject(lco.get());
            }

            return 0;
        }

        type = LuaScriptEngine::getType(-1);
    }

    return setPropertyFromStack(object, propertyName, type);
}

int LuaScriptEngine::setPropertyFromStack(osg::Object* object, const std::string& propertyName, osgDB::BaseSerializer::Type type) const
{
    switch(type)
    {
        case(osgDB::BaseSerializer::RW_BOOL):
        {
            if (lua_isboolean(_lua, -1))
            {
                _ci.setProperty(object, propertyName, static_cast<bool>(lua_toboolean(_lua, -1)!=0));
                return 0;
            }
            else if (lua_isnumber(_lua, -1))
            {
                _ci.setProperty(object, propertyName, static_cast<bool>(lua_tonumber(_lua, -1)!=0));
                return 0;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_STRING):
        {
            if (lua_isstring(_lua, -1))
            {
                _ci.setProperty(object, propertyName, std::string(lua_tostring(_lua, -1)));
                return 0;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_GLENUM):
        {
            if (lua_isnumber(_lua, -1))
            {
                _ci.setProperty(object, propertyName, static_cast<GLenum>(lua_tonumber(_lua, -1)));
                return 0;
            }
            else if (lua_isstring(_lua, -1))
            {
                const char* enumString = lua_tostring(_lua, -1);
                GLenum value = lookUpGLenumValue(enumString); //getValue("GL",enumString);

                _ci.setProperty(object, propertyName, value);
                return 0;
            }
            OSG_NOTICE<<"LuaScriptEngine::setPropertyFromStack("<<propertyName<<") osgDB::BaseSerializer::RW_GLENUM Failed"<<std::endl;
            break;
        }
        case(osgDB::BaseSerializer::RW_ENUM):
        {
            if (lua_isnumber(_lua, -1))
            {
                _ci.setProperty(object, propertyName, static_cast<int>(lua_tonumber(_lua, -1)));
                return 0;
            }
            else if (lua_isstring(_lua, -1))
            {
                const char* enumString = lua_tostring(_lua, -1);
                osgDB::BaseSerializer* serializer = _ci.getSerializer(object, propertyName, type);
                osgDB::IntLookup* lookup = serializer ? serializer->getIntLookup() : 0;
                if (lookup)
                {
                    int value = lookup->getValue(enumString);
                    _ci.setProperty(object, propertyName, value);
                }
                return 0;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_SHORT):
        {
            if (lua_isnumber(_lua, -1))
            {
                _ci.setProperty(object, propertyName, static_cast<short>(lua_tonumber(_lua, -1)));
                return 0;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_USHORT):
        {
            if (lua_isnumber(_lua, -1))
            {
                _ci.setProperty(object, propertyName, static_cast<unsigned short>(lua_tonumber(_lua, -1)));
                return 0;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_INT):
        {
            if (lua_isnumber(_lua, -1))
            {
                _ci.setProperty(object, propertyName, static_cast<int>(lua_tonumber(_lua, -1)));
                return 0;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_UINT):
        {
            if (lua_isnumber(_lua, -1))
            {
                _ci.setProperty(object, propertyName, static_cast<unsigned int>(lua_tonumber(_lua, -1)));
                return 0;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_FLOAT):
        {
            if (lua_isnumber(_lua, -1))
            {
                _ci.setProperty(object, propertyName, static_cast<float>(lua_tonumber(_lua, -1)));
                return 0;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_DOUBLE):
        {
            if (lua_isnumber(_lua, -1))
            {
                _ci.setProperty(object, propertyName, static_cast<double>(lua_tonumber(_lua, -1)));
                return 0;
            }
            break;
        }

        case(osgDB::BaseSerializer::RW_VEC2B): if (getValueAndSetProperty<osg::Vec2b>(object, propertyName)) return 0; break;
        case(osgDB::BaseSerializer::RW_VEC3B): if (getValueAndSetProperty<osg::Vec3b>(object, propertyName)) return 0; break;
        case(osgDB::BaseSerializer::RW_VEC4B): if (getValueAndSetProperty<osg::Vec4b>(object, propertyName)) return 0; break;

        case(osgDB::BaseSerializer::RW_VEC2UB): if (getValueAndSetProperty<osg::Vec2ub>(object, propertyName)) return 0; break;
        case(osgDB::BaseSerializer::RW_VEC3UB): if (getValueAndSetProperty<osg::Vec3ub>(object, propertyName)) return 0; break;
        case(osgDB::BaseSerializer::RW_VEC4UB): if (getValueAndSetProperty<osg::Vec4ub>(object, propertyName)) return 0; break;

        case(osgDB::BaseSerializer::RW_VEC2F): if (getValueAndSetProperty<osg::Vec2f>(object, propertyName)) return 0; break;
        case(osgDB::BaseSerializer::RW_VEC3F): if (getValueAndSetProperty<osg::Vec3f>(object, propertyName)) return 0; break;
        case(osgDB::BaseSerializer::RW_VEC4F): if (getValueAndSetProperty<osg::Vec4f>(object, propertyName)) return 0; break;

        case(osgDB::BaseSerializer::RW_VEC2D): if (getValueAndSetProperty<osg::Vec2d>(object, propertyName)) return 0; break;
        case(osgDB::BaseSerializer::RW_VEC3D): if (getValueAndSetProperty<osg::Vec3d>(object, propertyName)) return 0; break;
        case(osgDB::BaseSerializer::RW_VEC4D): if (getValueAndSetProperty<osg::Vec4d>(object, propertyName)) return 0; break;

        case(osgDB::BaseSerializer::RW_QUAT): if (getValueAndSetProperty<osg::Quat>(object, propertyName)) return 0; break;
        case(osgDB::BaseSerializer::RW_PLANE): if (getValueAndSetProperty<osg::Plane>(object, propertyName)) return 0; break;

#ifdef OSG_USE_FLOAT_MATRIX
        case(osgDB::BaseSerializer::RW_MATRIX):
#endif
        case(osgDB::BaseSerializer::RW_MATRIXF):
        {
            osg::Matrixd value;
            if (getValue(-1, value))
            {
                _ci.setProperty(object, propertyName, value);
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
            if (getValue(-1, value))
            {
                _ci.setProperty(object, propertyName, value);
                return 0;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_BOUNDINGBOXF):
        {
            osg::BoundingBoxf value;
            if (getValue(-1, value))
            {
                _ci.setProperty(object, propertyName, value);
                return 0;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_BOUNDINGBOXD):
        {
            osg::BoundingBoxd value;
            if (getValue(-1, value))
            {
                _ci.setProperty(object, propertyName, value);
                return 0;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_BOUNDINGSPHEREF):
        {
            osg::BoundingSpheref value;
            if (getValue(-1, value))
            {
                _ci.setProperty(object, propertyName, value);
                return 0;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_BOUNDINGSPHERED):
        {
            osg::BoundingSphered value;
            if (getValue(-1, value))
            {
                _ci.setProperty(object, propertyName, value);
                return 0;
            }
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
                    _ci.setProperty(object, propertyName, value);
                    return 0;
                }
                else
                {
                    OSG_NOTICE<<"Error: lua type '"<<lua_typename(_lua,lua_type(_lua, -1))<<"' cannot be assigned to "<<object->className()<<"::"<<propertyName<<std::endl;
                }
            }
            else if (lua_type(_lua,-1)==LUA_TFUNCTION)
            {
                int ref = luaL_ref(_lua, LUA_REGISTRYINDEX);
                osg::ref_ptr<LuaCallbackObject> lco = new LuaCallbackObject(propertyName, this, ref);
                osg::Object* value = lco.get();
                _ci.setProperty(object, propertyName, value);

                return 0;
            }
            else if (lua_isnil(_lua, -1))
            {
                OSG_NOTICE<<"Assigning property object (nil) to to object "<<object->className()<<"::"<<propertyName<<std::endl;
                osg::Object* value = 0;
                _ci.setProperty(object, propertyName, value);
                return 0;
            }
            else
            {
                OSG_NOTICE<<"Error: lua type '"<<lua_typename(_lua,lua_type(_lua, -1))<<"' cannot be assigned to "<<object->className()<<"::"<<propertyName<<std::endl;
                return 0;
            }
            break;
        }
        case(osgDB::BaseSerializer::RW_LIST):
        default:
            break;
    }
    OSG_NOTICE<<"LuaScriptEngine::setPropertyFromStack("<<object<<", "<<propertyName<<") property of type = "<<_ci.getTypeName(type)<<" not implemented"<<std::endl;
    return 0;
}



bool LuaScriptEngine::getfields(int pos, const char* f1, const char* f2, int type) const
{
    int abs_pos = getAbsolutePos(pos);
    lua_getfield(_lua, abs_pos, f1);
    lua_getfield(_lua, abs_pos, f2);
    if (lua_type(_lua, -2)!=type || lua_type(_lua, -1)!=type) { lua_pop(_lua, 2); return false; }

    return true;
}

bool LuaScriptEngine::getfields(int pos, const char* f1, const char* f2, const char* f3, int type) const
{
    int abs_pos = getAbsolutePos(pos);
    lua_getfield(_lua, abs_pos, f1);
    lua_getfield(_lua, abs_pos, f2);
    lua_getfield(_lua, abs_pos, f3);
    if (lua_type(_lua, -3)!=type || lua_type(_lua, -2)!=type ||
        lua_type(_lua, -1)!=type) { lua_pop(_lua, 3); return false; }

    return true;
}

bool LuaScriptEngine::getfields(int pos, const char* f1, const char* f2, const char* f3, const char* f4, int type) const
{
    int abs_pos = getAbsolutePos(pos);
    lua_getfield(_lua, abs_pos, f1);
    lua_getfield(_lua, abs_pos, f2);
    lua_getfield(_lua, abs_pos, f3);
    lua_getfield(_lua, abs_pos, f4);
    if (lua_type(_lua, -4)!=type || lua_type(_lua, -3)!=type ||
        lua_type(_lua, -2)!=type || lua_type(_lua, -1)!=type) { lua_pop(_lua, 4); return false; }

    return true;
}

bool LuaScriptEngine::getfields(int pos, const char* f1, const char* f2, const char* f3, const char* f4, const char* f5, const char* f6, int type) const
{
    int abs_pos = getAbsolutePos(pos);
    lua_getfield(_lua, abs_pos, f1);
    lua_getfield(_lua, abs_pos, f2);
    lua_getfield(_lua, abs_pos, f3);
    lua_getfield(_lua, abs_pos, f4);
    lua_getfield(_lua, abs_pos, f5);
    lua_getfield(_lua, abs_pos, f6);
    if (lua_type(_lua, -6)!=type || lua_type(_lua, -5)!=type ||
        lua_type(_lua, -4)!=type || lua_type(_lua, -3)!=type ||
        lua_type(_lua, -2)!=type || lua_type(_lua, -1)!=type) { lua_pop(_lua, 6); return false; }

    return true;
}

bool LuaScriptEngine::getelements(int pos, int numElements, int type) const
{
    int abs_pos = getAbsolutePos(pos);
    for(int i=0; i<numElements; ++i)
    {
        lua_pushinteger(_lua, i);
        lua_gettable(_lua, abs_pos);
        if (lua_type(_lua, -1)!=type) { lua_pop(_lua, i+1); return false; }
    }
    return true;
}


osgDB::BaseSerializer::Type LuaScriptEngine::getType(int pos) const
{
    int abs_pos = getAbsolutePos(pos);
    switch(lua_type(_lua, abs_pos))
    {
        case(LUA_TNIL): return osgDB::BaseSerializer::RW_UNDEFINED;
        case(LUA_TNUMBER): return osgDB::BaseSerializer::RW_DOUBLE;
        case(LUA_TBOOLEAN): return osgDB::BaseSerializer::RW_BOOL;
        case(LUA_TSTRING): return osgDB::BaseSerializer::RW_STRING;
        case(LUA_TTABLE):
        {
            lua_pushstring(_lua, "object_ptr");
            lua_rawget(_lua, abs_pos);
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
                return osgDB::BaseSerializer::RW_VEC2D;
            }
            else if ((numStringKeys==3 || numNumberKeys==3) && (numNumberFields==3))
            {
                return osgDB::BaseSerializer::RW_VEC3D;
            }
            else if ((numStringKeys==4 || numNumberKeys==4) && (numNumberFields==4))
            {
                return osgDB::BaseSerializer::RW_VEC4D;
            }
            else if ((numNumberKeys==16) && (numNumberFields==16))
            {
                return osgDB::BaseSerializer::RW_MATRIXD;
            }
            else if ((numNumberKeys==6) && (numNumberFields==6))
            {
                return osgDB::BaseSerializer::RW_BOUNDINGBOXD;
            }
            // not supported
            OSG_NOTICE<<"Warning: LuaScriptEngine::getType() Lua table configuration not supported."<<std::endl;
            break;
        }
        default:
            OSG_NOTICE<<"Warning: LuaScriptEngine::getType() Lua type "<<lua_typename(_lua, lua_type(_lua, abs_pos))<<" not supported."<<std::endl;
            break;

    }
    return osgDB::BaseSerializer::RW_UNDEFINED;
}

bool LuaScriptEngine::getvec2(int pos) const
{
    int abs_pos = getAbsolutePos(pos);
    if (lua_istable(_lua, abs_pos))
    {
        if (getfields(abs_pos, "x", "y", LUA_TNUMBER) ||
            getfields(abs_pos, "s", "t", LUA_TNUMBER) ||
            getfields(abs_pos, "luminance", "alpha", LUA_TNUMBER) ||
            getelements(abs_pos, 2, LUA_TNUMBER))
        {
            return true;
        }
    }
    return false;
}

bool LuaScriptEngine::getvec3(int pos) const
{
    int abs_pos = getAbsolutePos(pos);
    if (lua_istable(_lua, abs_pos))
    {
        if (getfields(abs_pos, "x", "y", "z", LUA_TNUMBER) ||
            getfields(abs_pos, "r", "g", "b", LUA_TNUMBER) ||
            getfields(abs_pos, "red", "green", "blue", LUA_TNUMBER) ||
            getfields(abs_pos, "s", "t", "r", LUA_TNUMBER) ||
            getelements(abs_pos, 3, LUA_TNUMBER))
        {
            return true;
        }
    }
    return false;
}
bool LuaScriptEngine::getvec4(int pos) const
{
    int abs_pos = getAbsolutePos(pos);
    if (lua_istable(_lua, abs_pos))
    {
        if (getfields(abs_pos, "x", "y", "z", "w", LUA_TNUMBER) ||
            getfields(abs_pos, "r", "g", "b", "a", LUA_TNUMBER) ||
            getfields(abs_pos, "red", "green", "blue", "alpha", LUA_TNUMBER) ||
            getfields(abs_pos, "s", "t", "r", "q", LUA_TNUMBER) ||
            getelements(abs_pos, 4, LUA_TNUMBER))
        {
            return true;
        }
    }
    return false;
}

bool LuaScriptEngine::getmatrix(int pos) const
{
    int abs_pos = getAbsolutePos(pos);
    if (lua_istable(_lua, abs_pos))
    {
        if (getelements(abs_pos, 16,LUA_TNUMBER))
        {
            return true;
        }
    }
    return false;
}

bool LuaScriptEngine::getboundingbox(int pos) const
{
    int abs_pos = getAbsolutePos(pos);
    if (lua_istable(_lua, abs_pos))
    {
        if (getfields(abs_pos, "xMin", "yMin", "zMin", "xMax", "yMax", "zMax", LUA_TNUMBER) ||
            getelements(abs_pos, 6, LUA_TNUMBER))
        {
            return true;
        }
    }
    return false;
}

bool LuaScriptEngine::getboundingsphere(int pos) const
{
    int abs_pos = getAbsolutePos(pos);
    if (lua_istable(_lua, abs_pos))
    {
        if (getfields(abs_pos, "x", "y", "z", "radius", LUA_TNUMBER) ||
            getelements(abs_pos, 4, LUA_TNUMBER))
        {
            return true;
        }
    }
    return false;
}

bool LuaScriptEngine::getValue(int pos, osg::Matrixf& value) const
{
    if (!getmatrix(pos)) return false;

    for(int r=0; r<4; ++r)
    {
        for(int c=0; c<4; ++c)
        {
            value(r,c) = lua_tonumber(_lua, -16+(r*4+c));
        }
    }
    lua_pop(_lua, 16);
    return true;
}

bool LuaScriptEngine::getValue(int pos, osg::Matrixd& value) const
{
    if (!getmatrix(pos)) return false;

    for(int r=0; r<4; ++r)
    {
        for(int c=0; c<4; ++c)
        {
            value(r,c) = lua_tonumber(_lua, -16+(r*4+c));
        }
    }
    lua_pop(_lua, 16);
    return true;
}

bool LuaScriptEngine::getValue(int pos, osg::BoundingBoxf& value) const
{
    if (!getboundingbox(pos)) return false;
    value.set(lua_tonumber(_lua, -6), lua_tonumber(_lua, -5), lua_tonumber(_lua, -4), lua_tonumber(_lua, -3), lua_tonumber(_lua, -2), lua_tonumber(_lua, -1));
    lua_pop(_lua, 6);
    return true;
}

bool LuaScriptEngine::getValue(int pos, osg::BoundingBoxd& value) const
{
    if (!getboundingbox(pos)) return false;
    value.set(lua_tonumber(_lua, -6), lua_tonumber(_lua, -5), lua_tonumber(_lua, -4), lua_tonumber(_lua, -3), lua_tonumber(_lua, -2), lua_tonumber(_lua, -1));
    lua_pop(_lua, 6);
    return true;
}

bool LuaScriptEngine::getValue(int pos, osg::BoundingSpheref& value) const
{
    if (!getboundingsphere(pos)) return false;
    value.set(osg::Vec3f(lua_tonumber(_lua, -4), lua_tonumber(_lua, -3), lua_tonumber(_lua, -2)), lua_tonumber(_lua, -1));
    lua_pop(_lua, 4);
    return true;
}

bool LuaScriptEngine::getValue(int pos, osg::BoundingSphered& value) const
{
    if (!getboundingsphere(pos)) return false;
    value.set(osg::Vec3d(lua_tonumber(_lua, -4), lua_tonumber(_lua, -3), lua_tonumber(_lua, -2)), lua_tonumber(_lua, -1));
    lua_pop(_lua, 4);
    return true;
}

void LuaScriptEngine::pushValue(const osg::Matrixf& value) const
{
    lua_newtable(_lua);
    lua_newtable(_lua); luaL_getmetatable(_lua, "LuaScriptEngine.Table"); lua_setmetatable(_lua, -2);

    for(unsigned int r=0; r<4; ++r)
    {
        for(unsigned int c=0; c<4; ++c)
        {
            lua_pushinteger(_lua, r*4+c); lua_pushnumber(_lua, (lua_Integer) value(r,c)); lua_settable(_lua, -3);
        }
    }
}

void LuaScriptEngine::pushValue(const osg::Matrixd& value) const
{
    lua_newtable(_lua);
    lua_newtable(_lua); luaL_getmetatable(_lua, "LuaScriptEngine.Table"); lua_setmetatable(_lua, -2);

    for(unsigned int r=0; r<4; ++r)
    {
        for(unsigned int c=0; c<4; ++c)
        {
            lua_pushinteger(_lua, r*4+c); lua_pushnumber(_lua, value(r,c)); lua_settable(_lua, -3);
        }
    }
}

void LuaScriptEngine::pushValue(const osg::BoundingBoxf& value) const
{
    lua_newtable(_lua);
    lua_newtable(_lua); luaL_getmetatable(_lua, "LuaScriptEngine.Table"); lua_setmetatable(_lua, -2);
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
    lua_newtable(_lua); luaL_getmetatable(_lua, "LuaScriptEngine.Table"); lua_setmetatable(_lua, -2);
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
    lua_newtable(_lua); luaL_getmetatable(_lua, "LuaScriptEngine.Table"); lua_setmetatable(_lua, -2);
    lua_pushstring(_lua, "x"); lua_pushnumber(_lua, value.center().x()); lua_settable(_lua, -3);
    lua_pushstring(_lua, "y"); lua_pushnumber(_lua, value.center().y()); lua_settable(_lua, -3);
    lua_pushstring(_lua, "z"); lua_pushnumber(_lua, value.center().z()); lua_settable(_lua, -3);
    lua_pushstring(_lua, "radius"); lua_pushnumber(_lua, value.radius()); lua_settable(_lua, -3);
}

void LuaScriptEngine::pushValue(const osg::BoundingSphered& value) const
{
    lua_newtable(_lua);
    lua_newtable(_lua); luaL_getmetatable(_lua, "LuaScriptEngine.Table"); lua_setmetatable(_lua, -2);
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

    osgDB::BaseSerializer::Type type = getType(-1);
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
        case(osgDB::BaseSerializer::RW_GLENUM):
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

        case(osgDB::BaseSerializer::RW_VEC2B): object = getValueObject<osg::Vec2b>(-1); break;
        case(osgDB::BaseSerializer::RW_VEC3B): object = getValueObject<osg::Vec3b>(-1); break;
        case(osgDB::BaseSerializer::RW_VEC4B): object = getValueObject<osg::Vec4b>(-1); break;

        case(osgDB::BaseSerializer::RW_VEC2UB): object = getValueObject<osg::Vec2ub>(-1); break;
        case(osgDB::BaseSerializer::RW_VEC3UB): object = getValueObject<osg::Vec3ub>(-1); break;
        case(osgDB::BaseSerializer::RW_VEC4UB): object = getValueObject<osg::Vec4ub>(-1); break;

        case(osgDB::BaseSerializer::RW_VEC2S): object = getValueObject<osg::Vec2s>(-1); break;
        case(osgDB::BaseSerializer::RW_VEC3S): object = getValueObject<osg::Vec3s>(-1); break;
        case(osgDB::BaseSerializer::RW_VEC4S): object = getValueObject<osg::Vec4s>(-1); break;

        case(osgDB::BaseSerializer::RW_VEC2US): object = getValueObject<osg::Vec2us>(-1); break;
        case(osgDB::BaseSerializer::RW_VEC3US): object = getValueObject<osg::Vec3us>(-1); break;
        case(osgDB::BaseSerializer::RW_VEC4US): object = getValueObject<osg::Vec4us>(-1); break;

        case(osgDB::BaseSerializer::RW_VEC2I): object = getValueObject<osg::Vec2i>(-1); break;
        case(osgDB::BaseSerializer::RW_VEC3I): object = getValueObject<osg::Vec3i>(-1); break;
        case(osgDB::BaseSerializer::RW_VEC4I): object = getValueObject<osg::Vec4i>(-1); break;

        case(osgDB::BaseSerializer::RW_VEC2UI): object = getValueObject<osg::Vec2ui>(-1); break;
        case(osgDB::BaseSerializer::RW_VEC3UI): object = getValueObject<osg::Vec3ui>(-1); break;
        case(osgDB::BaseSerializer::RW_VEC4UI): object = getValueObject<osg::Vec4ui>(-1); break;

        case(osgDB::BaseSerializer::RW_VEC2F): object = getValueObject<osg::Vec2f>(-1); break;
        case(osgDB::BaseSerializer::RW_VEC3F): object = getValueObject<osg::Vec3f>(-1); break;
        case(osgDB::BaseSerializer::RW_VEC4F): object = getValueObject<osg::Vec4f>(-1); break;

        case(osgDB::BaseSerializer::RW_VEC2D): object = getValueObject<osg::Vec2d>(-1); break;
        case(osgDB::BaseSerializer::RW_VEC3D): object = getValueObject<osg::Vec3d>(-1); break;
        case(osgDB::BaseSerializer::RW_VEC4D): object = getValueObject<osg::Vec4d>(-1); break;

        case(osgDB::BaseSerializer::RW_QUAT): object = getValueObject<osg::Quat>(-1); break;
        case(osgDB::BaseSerializer::RW_PLANE): object = getValueObject<osg::Plane>(-1); break;

#ifdef OSG_USE_FLOAT_MATRIX
        case(osgDB::BaseSerializer::RW_MATRIX):
#endif
        case(osgDB::BaseSerializer::RW_MATRIXF):
        {
            osg::Matrixf value;
            if (getValue(-1, value)) object = new osg::MatrixfValueObject("", value);
            break;
        }

#ifndef OSG_USE_FLOAT_MATRIX
        case(osgDB::BaseSerializer::RW_MATRIX):
#endif
        case(osgDB::BaseSerializer::RW_MATRIXD):
        {
            osg::Matrixd value;
            if (getValue(-1, value)) object = new osg::MatrixdValueObject("", value);
            break;
        }
        case(osgDB::BaseSerializer::RW_BOUNDINGBOXF):
        {
            osg::BoundingBoxf value;
            if (getValue(-1, value)) object = new osg::BoundingBoxfValueObject("", value);
            break;
        }
        case(osgDB::BaseSerializer::RW_BOUNDINGBOXD):
        {
            osg::BoundingBoxd value;
            if (getValue(-1, value)) object = new osg::BoundingBoxdValueObject("", value);
            break;
        }
        case(osgDB::BaseSerializer::RW_BOUNDINGSPHEREF):
        {
            osg::BoundingSpheref value;
            if (getValue(-1, value)) object = new osg::BoundingSpherefValueObject("", value);
            break;
        }
        case(osgDB::BaseSerializer::RW_BOUNDINGSPHERED):
        {
            osg::BoundingSphered value;
            if (getValue(-1, value)) object = new osg::BoundingSpheredValueObject("", value);
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
}

void LuaScriptEngine::pushContainer(osg::Object* object, const std::string& propertyName) const
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

        lua_pushstring(_lua, "containerPropertyName"); lua_pushstring(_lua, propertyName.c_str()); lua_settable(_lua, -3);

        osgDB::BaseSerializer::Type type;
        osgDB::BaseSerializer* bs = _ci.getSerializer(object, propertyName, type);
        osgDB::VectorBaseSerializer* vs = dynamic_cast<osgDB::VectorBaseSerializer*>(bs);
        osgDB::MapBaseSerializer* ms = dynamic_cast<osgDB::MapBaseSerializer*>(bs);
        if (vs)
        {
            assignClosure("size", getContainerSize);
            assignClosure("clear", callVectorClear);
            assignClosure("resize", callVectorResize);
            assignClosure("reserve", callVectorReserve);
            assignClosure("add", callVectorAdd);

            luaL_getmetatable(_lua, "LuaScriptEngine.Container");
            lua_setmetatable(_lua, -2);
        }
        else if (ms)
        {
            assignClosure("clear", callMapClear);
            assignClosure("size", getMapSize);
            assignClosure("createIterator", createMapIterator);
            assignClosure("createReverseIterator", createMapReverseIterator);

            luaL_getmetatable(_lua, "LuaScriptEngine.Map");
            lua_setmetatable(_lua, -2);
        }
        else
        {
            OSG_NOTICE<<"Container type not supported."<<std::endl;
        }
    }
    else
    {
        lua_pushnil(_lua);
    }
}


void LuaScriptEngine::createAndPushObject(const std::string& compoundName) const
{
    osg::ref_ptr<osg::Object> object = _ci.createObject(compoundName);
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

        // check to see if Object "is a" vector
        osgDB::BaseSerializer::Type type;
        osgDB::BaseSerializer* vs = _ci.getSerializer(object, "vector", type);
        if (vs)
        {
            lua_pushstring(_lua, "containerPropertyName"); lua_pushstring(_lua, "vector"); lua_settable(_lua, -3);

            assignClosure("size", getContainerSize);
            assignClosure("clear", callVectorClear);
            assignClosure("resize", callVectorResize);
            assignClosure("reserve", callVectorReserve);
            assignClosure("add", callVectorAdd);

            luaL_getmetatable(_lua, "LuaScriptEngine.Container");
            lua_setmetatable(_lua, -2);
        }
        else if (dynamic_cast<osgDB::MapIteratorObject*>(object)!=0)
        {
            assignClosure("advance", callMapIteratorAdvance);
            assignClosure("valid", callMapIteratorValid);
            assignClosure("getKey", getMapIteratorKey);
            assignClosure("getElement", getMapIteratorElement);
            assignClosure("setElement", setMapIteratorElement);
        }
        else if (dynamic_cast<osg::Image*>(object)!=0)
        {
            assignClosure("allocate", callImageAllocate);
            assignClosure("s", callImageS);
            assignClosure("t", callImageT);
            assignClosure("r", callImageR);
            assignClosure("get", callImageGet);
            assignClosure("set", callImageSet);

            luaL_getmetatable(_lua, "LuaScriptEngine.Object");
            lua_setmetatable(_lua, -2);
        }
        else if (dynamic_cast<osg::StateSet*>(object)!=0)
        {
            assignClosure("add", callStateSetSet);
            assignClosure("set", callStateSetSet);
            assignClosure("get", callStateSetGet);
            assignClosure("remove", callStateSetRemove);

            luaL_getmetatable(_lua, "LuaScriptEngine.Object");
            lua_setmetatable(_lua, -2);
        }
        else if (dynamic_cast<osg::Node*>(object)!=0)
        {
            assignClosure("getParent", callGetParent);
            assignClosure("getNumParents", callGetNumParents);

            luaL_getmetatable(_lua, "LuaScriptEngine.Object");
            lua_setmetatable(_lua, -2);
        }
        else
        {
            luaL_getmetatable(_lua, "LuaScriptEngine.Object");
            lua_setmetatable(_lua, -2);
        }
    }
    else
    {
        lua_pushnil(_lua);
    }
}

void LuaScriptEngine::pushAndCastObject(const std::string& compoundClassName, osg::Object* object) const
{
    if (object && _ci.isObjectOfType(object, compoundClassName))
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

        std::string::size_type separator = compoundClassName.find("::");
        std::string libraryName = (separator==std::string::npos) ? object->libraryName() : compoundClassName.substr(0, separator);
        std::string className = (separator==std::string::npos) ? object->className() : compoundClassName.substr(separator+2,std::string::npos);

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

void LuaScriptEngine::addPaths(const osgDB::FilePathList& paths)
{
    lua_getglobal( _lua, "package" );

    lua_getfield( _lua, -1, "path" );
    std::string  path = lua_tostring( _lua, -1 );
    lua_pop( _lua, 1 );

    OSG_INFO<<"LuaScriptEngine::addPaths() original package.path = "<<path<<std::endl;


    for(osgDB::FilePathList::const_iterator itr = paths.begin();
        itr != paths.end();
        ++itr)
    {
        OSG_INFO<<"  Appending path ["<<*itr<<"]"<<std::endl;

        path.append( ";" );
        path.append( *itr );
        path.append( "/?.lua" );
    }

    OSG_INFO<<"   path after = "<<path<<std::endl;

    lua_pushstring( _lua, path.c_str() );
    lua_setfield( _lua, -2, "path" );

    lua_pop( _lua, 1 ); // return stack to original
}

void LuaScriptEngine::addPaths(const osgDB::Options* options)
{
    if (!options) return;
    addPaths(options->getDatabasePathList());
}

