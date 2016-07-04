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
#include <osgDB/ClassInterface>

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

namespace lua
{

// forward declare
class LuaScriptEngine;

struct SerializerScratchPad : public osg::Referenced
{
    SerializerScratchPad(unsigned int s=256) : deleteData(true), dataType(osgDB::BaseSerializer::RW_UNDEFINED), dataSize(0) { maxDataSize = s; data = new char[s]; }
    SerializerScratchPad(osgDB::BaseSerializer::Type type, const void* ptr, unsigned int s) : deleteData(false), maxDataSize(s), data(const_cast<char*>(reinterpret_cast<const char*>(ptr))), dataType(type),dataSize(s) {}
    virtual ~SerializerScratchPad() { if (deleteData && data) delete [] data; }

    bool                        deleteData;
    unsigned int                maxDataSize;
    char*                       data;

    osgDB::BaseSerializer::Type dataType;
    unsigned int                dataSize;

    void reset()
    {
        dataType = osgDB::BaseSerializer::RW_UNDEFINED;
        dataSize = 0;
    }

    template<typename T>
    bool set(const T& t)
    {
        if (sizeof(T)<=maxDataSize)
        {
            *(reinterpret_cast<T*>(data)) = t;
            dataType = osgDB::getTypeEnum<T>();
            dataSize = sizeof(T);
            return true;
        }
        else
        {
            dataSize = 0;
            dataType = osgDB::BaseSerializer::RW_UNDEFINED;
            return false;
        }
    }

    template<typename T>
    bool get(T& t) const
    {
        if (sizeof(T)==dataSize && dataType == osgDB::getTypeEnum<T>())
        {
            t = *(reinterpret_cast<T*>(data));
            return true;
        }
        else
        {
            return false;
        }
    }
};


class LuaScriptEngine : public osg::ScriptEngine
{
    public:
        LuaScriptEngine();
        LuaScriptEngine(const LuaScriptEngine& rhs, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);

        META_Object(lua, LuaScriptEngine)

        virtual const std::string& getLanguage() const { return _language; }

        /** run a Script.*/
        virtual bool run(osg::Script* script, const std::string& entryPoint, osg::Parameters& inputParameters, osg::Parameters& outputParameters);

        /** get the lua_State object.*/
        lua_State* getLuaState() const { return _lua; }

        osgDB::ClassInterface& getClassInterface() const { return _ci; }

        int pushDataToStack(SerializerScratchPad* ssp) const;

        template<typename T>
        bool pushValueToStack(SerializerScratchPad* ssp) const
        {
            T value;
            if (ssp->get(value))
            {
                pushValue(value);
                return true;
            }
            return false;
        }

        int getDataFromStack(SerializerScratchPad* ssp, osgDB::BaseSerializer::Type type, int pos) const;

        template<typename T>
        bool getDataFromStack(SerializerScratchPad* ssp, int pos) const
        {
            T value;
            if (getValue(pos, value))
            {
                ssp->set(value);
                return true;
            }
            return false;
        }

        int pushPropertyToStack(osg::Object* object, const std::string& propertyName) const;
        int setPropertyFromStack(osg::Object* object, const std::string& propertyName) const;
        int setPropertyFromStack(osg::Object* object, const std::string& propertyName, osgDB::BaseSerializer::Type type) const;

        bool loadScript(osg::Script* script);

        int getAbsolutePos(int pos) const { return (pos<0) ? (lua_gettop(_lua)+pos+1) : pos; }

        osgDB::BaseSerializer::Type getType(int pos) const;

        bool getfields(int pos, const char* f1, const char* f2, int type) const;
        bool getfields(int pos, const char* f1, const char* f2, const char* f3, int type) const;
        bool getfields(int pos, const char* f1, const char* f2, const char* f3, const char* f4, int type) const;
        bool getfields(int pos, const char* f1, const char* f2, const char* f3, const char* f4, const char* f5, const char* f6, int type) const;
        bool getelements(int pos, int numElements, int type) const;

        bool getvec2(int pos) const;
        bool getvec3(int pos) const;
        bool getvec4(int pos) const;
        bool getmatrix(int pos) const;
        bool getboundingbox(int pos) const;
        bool getboundingsphere(int pos) const;




        template<typename T>
        bool getVec2(int pos, T& value) const
        {
            if (!getvec2(pos)) return false;

            value.set(lua_tonumber(_lua, -2), lua_tonumber(_lua, -1));
            lua_pop(_lua, 2);

            return true;
        }

        template<typename T>
        bool getVec3(int pos, T& value) const
        {
            if (!getvec3(pos)) return false;
            value.set(lua_tonumber(_lua, -3), lua_tonumber(_lua, -2), lua_tonumber(_lua, -1));
            lua_pop(_lua, 3);
            return true;
        }

        template<typename T>
        bool getVec4(int pos, T& value) const
        {
            if (!getvec4(pos)) return false;
            value.set(lua_tonumber(_lua, -4), lua_tonumber(_lua, -3), lua_tonumber(_lua, -2), lua_tonumber(_lua, -1));
            lua_pop(_lua, 4);
            return true;
        }

        bool getValue(int pos, osg::Vec2b& value) const { return getVec2(pos, value); }
        bool getValue(int pos, osg::Vec3b& value) const { return getVec3(pos, value); }
        bool getValue(int pos, osg::Vec4b& value) const { return getVec4(pos, value); }

        bool getValue(int pos, osg::Vec2ub& value) const { return getVec2(pos, value); }
        bool getValue(int pos, osg::Vec3ub& value) const { return getVec3(pos, value); }
        bool getValue(int pos, osg::Vec4ub& value) const { return getVec4(pos, value); }

        bool getValue(int pos, osg::Vec2s& value) const { return getVec2(pos, value); }
        bool getValue(int pos, osg::Vec3s& value) const { return getVec3(pos, value); }
        bool getValue(int pos, osg::Vec4s& value) const { return getVec4(pos, value); }

        bool getValue(int pos, osg::Vec2us& value) const { return getVec2(pos, value); }
        bool getValue(int pos, osg::Vec3us& value) const { return getVec3(pos, value); }
        bool getValue(int pos, osg::Vec4us& value) const { return getVec4(pos, value); }

        bool getValue(int pos, osg::Vec2i& value) const { return getVec2(pos, value); }
        bool getValue(int pos, osg::Vec3i& value) const { return getVec3(pos, value); }
        bool getValue(int pos, osg::Vec4i& value) const { return getVec4(pos, value); }

        bool getValue(int pos, osg::Vec2ui& value) const { return getVec2(pos, value); }
        bool getValue(int pos, osg::Vec3ui& value) const { return getVec3(pos, value); }
        bool getValue(int pos, osg::Vec4ui& value) const { return getVec4(pos, value); }

        bool getValue(int pos, osg::Vec2f& value) const { return getVec2(pos, value); }
        bool getValue(int pos, osg::Vec3f& value) const { return getVec3(pos, value); }
        bool getValue(int pos, osg::Vec4f& value) const { return getVec4(pos, value); }

        bool getValue(int pos, osg::Vec2d& value) const { return getVec2(pos, value); }
        bool getValue(int pos, osg::Vec3d& value) const { return getVec3(pos, value); }
        bool getValue(int pos, osg::Vec4d& value) const { return getVec4(pos, value); }

        bool getValue(int pos, osg::Quat& value) const { return getVec4(pos, value); }
        bool getValue(int pos, osg::Plane& value) const { return getVec4(pos, value); }

        bool getValue(int pos, osg::Matrixf& value) const;
        bool getValue(int pos, osg::Matrixd& value) const;

        bool getValue(int pos, osg::BoundingBoxf& value) const;
        bool getValue(int pos, osg::BoundingBoxd& value) const;

        bool getValue(int pos, osg::BoundingSpheref& value) const;
        bool getValue(int pos, osg::BoundingSphered& value) const;


        template<typename T>
        bool getValueAndSetProperty(osg::Object* object, const std::string& propertyName) const
        {
            T value;
            if (getValue(-1, value))
            {
                _ci.setProperty(object, propertyName, value);
                return true;
            }
            return false;
        }

        template<typename T>
        osg::Object* getValueObject(int pos) const
        {
            T value;
            if (getValue(pos, value)) return new osg::TemplateValueObject<T>("", value);
            else return 0;
        }


        template<typename T>
        bool getPropertyAndPushValue(const osg::Object* object, const std::string& propertyName) const
        {
            T value;
            if (_ci.getProperty(object, propertyName, value))
            {
                pushValue(value);
                return true;
            }
            return false;
        }


        void pushValue(osgDB::BaseSerializer::Type type, const void* ptr) const;

        template<typename T>
        void pushVec2(const T& value) const
        {
            lua_newtable(_lua);
            lua_newtable(_lua); luaL_getmetatable(_lua, "LuaScriptEngine.Table"); lua_setmetatable(_lua, -2);
            lua_pushstring(_lua, "x"); lua_pushnumber(_lua, value.x()); lua_settable(_lua, -3);
            lua_pushstring(_lua, "y"); lua_pushnumber(_lua, value.y()); lua_settable(_lua, -3);
        }

        template<typename T>
        void pushVec3(const T& value) const
        {
            lua_newtable(_lua);
            lua_newtable(_lua); luaL_getmetatable(_lua, "LuaScriptEngine.Table"); lua_setmetatable(_lua, -2);
            lua_pushstring(_lua, "x"); lua_pushnumber(_lua, value.x()); lua_settable(_lua, -3);
            lua_pushstring(_lua, "y"); lua_pushnumber(_lua, value.y()); lua_settable(_lua, -3);
            lua_pushstring(_lua, "z"); lua_pushnumber(_lua, value.z()); lua_settable(_lua, -3);
        }

        template<typename T>
        void pushVec4(const T& value) const
        {
            lua_newtable(_lua);
            lua_newtable(_lua); luaL_getmetatable(_lua, "LuaScriptEngine.Table"); lua_setmetatable(_lua, -2);
            lua_pushstring(_lua, "x"); lua_pushnumber(_lua, value.x()); lua_settable(_lua, -3);
            lua_pushstring(_lua, "y"); lua_pushnumber(_lua, value.y()); lua_settable(_lua, -3);
            lua_pushstring(_lua, "z"); lua_pushnumber(_lua, value.z()); lua_settable(_lua, -3);
            lua_pushstring(_lua, "w"); lua_pushnumber(_lua, value.w()); lua_settable(_lua, -3);
        }

        void pushValue(const osg::Vec2b& value) const { pushVec2(value); }
        void pushValue(const osg::Vec3b& value) const { pushVec3(value); }
        void pushValue(const osg::Vec4b& value) const { pushVec4(value); }

        void pushValue(const osg::Vec2ub& value) const { pushVec2(value); }
        void pushValue(const osg::Vec3ub& value) const { pushVec3(value); }
        void pushValue(const osg::Vec4ub& value) const { pushVec4(value); }

        void pushValue(const osg::Vec2s& value) const { pushVec2(value); }
        void pushValue(const osg::Vec3s& value) const { pushVec3(value); }
        void pushValue(const osg::Vec4s& value) const { pushVec4(value); }

        void pushValue(const osg::Vec2us& value) const { pushVec2(value); }
        void pushValue(const osg::Vec3us& value) const { pushVec3(value); }
        void pushValue(const osg::Vec4us& value) const { pushVec4(value); }

        void pushValue(const osg::Vec2i& value) const { pushVec2(value); }
        void pushValue(const osg::Vec3i& value) const { pushVec3(value); }
        void pushValue(const osg::Vec4i& value) const { pushVec4(value); }

        void pushValue(const osg::Vec2ui& value) const { pushVec2(value); }
        void pushValue(const osg::Vec3ui& value) const { pushVec3(value); }
        void pushValue(const osg::Vec4ui& value) const { pushVec4(value); }

        void pushValue(const osg::Vec2f& value) const { pushVec2(value); }
        void pushValue(const osg::Vec3f& value) const { pushVec3(value); }
        void pushValue(const osg::Vec4f& value) const { pushVec4(value); }

        void pushValue(const osg::Vec2d& value) const { pushVec2(value); }
        void pushValue(const osg::Vec3d& value) const { pushVec3(value); }
        void pushValue(const osg::Vec4d& value) const { pushVec4(value); }

        void pushValue(const osg::Quat& value) const { pushVec4(value); }
        void pushValue(const osg::Plane& value) const { pushVec4(value.asVec4()); }

        void pushValue(const osg::Matrixf& value) const;
        void pushValue(const osg::Matrixd& value) const;

        void pushValue(const osg::BoundingBoxf& value) const;
        void pushValue(const osg::BoundingBoxd& value) const;

        void pushValue(const osg::BoundingSpheref& value) const;
        void pushValue(const osg::BoundingSphered& value) const;

        bool pushParameter(osg::Object* object) const;
        bool popParameter(osg::Object* object) const;
        osg::Object* popParameterObject() const;

        void pushContainer(osg::Object* object, const std::string& propertyName) const;

        void createAndPushObject(const std::string& compoundName) const;
        void pushObject(osg::Object* object) const;
        void pushAndCastObject(const std::string& compoundClassName, osg::Object* object) const;

        template<class T>
        T* getObjectFromTable(int pos) const
        {
            if (lua_type(_lua, pos)==LUA_TTABLE)
            {
                lua_pushstring(_lua, "object_ptr");
                lua_rawget(_lua, pos);

                osg::Object* object = (lua_type(_lua, -1)==LUA_TUSERDATA)?
                    *const_cast<osg::Object**>(reinterpret_cast<const osg::Object**>(lua_touserdata(_lua,-1))) :
                    0;

                lua_pop(_lua,1);

                return dynamic_cast<T*>(object);
            }
            else return 0;
        }

        std::string getStringFromTable(int pos, const std::string& field) const
        {
            std::string result;
            if (lua_type(_lua, pos)==LUA_TTABLE)
            {
                lua_pushstring(_lua, field.c_str());
                lua_rawget(_lua, pos);

                if (lua_type(_lua, -1)==LUA_TSTRING)
                {
                    result = lua_tostring(_lua, -1);
                }

                lua_pop(_lua,1);
            }
            return result;
        }

        std::string getObjectCompoundClassName(int pos) const
        {
            if (lua_type(_lua, pos)==LUA_TTABLE)
            {
                lua_pushstring(_lua, "compoundClassName");
                lua_rawget(_lua, pos);

                std::string compoundClassName = lua_tostring(_lua, -1);

                lua_pop(_lua,1);

                return compoundClassName;
            }
            else return std::string("");
        }

        void assignClosure(const char* name, lua_CFunction fn) const;

        bool matchLuaParameters(int luaType1) const { return ((lua_gettop(_lua)==1) && (lua_type(_lua, 1)==luaType1)); }
        bool matchLuaParameters(int luaType1, int luaType2) const { return ((lua_gettop(_lua)==2) && (lua_type(_lua, 1)==luaType1) && (lua_type(_lua, 2)==luaType2)); }
        bool matchLuaParameters(int luaType1, int luaType2, int luaType3) const { return ((lua_gettop(_lua)==3) && (lua_type(_lua, 1)==luaType1) && (lua_type(_lua, 2)==luaType2) && (lua_type(_lua, 3)==luaType3)); }
        bool matchLuaParameters(int luaType1, int luaType2, int luaType3, int luaType4) const { return ((lua_gettop(_lua)==4) && (lua_type(_lua, 1)==luaType1) && (lua_type(_lua, 2)==luaType2) && (lua_type(_lua, 3)==luaType3) && (lua_type(_lua, 4)==luaType4)); }

        std::string lookUpGLenumString(GLenum value) const;
        GLenum lookUpGLenumValue(const std::string& str) const;


        void addPaths(const osgDB::FilePathList& paths);
        void addPaths(const osgDB::Options* options);

    protected:

        void initialize();

        virtual ~LuaScriptEngine();

        lua_State* _lua;

        unsigned int _scriptCount;
        std::string createUniquieScriptName();

        typedef std::map< osg::ref_ptr<osg::Script>, std::string> ScriptMap;
        ScriptMap _loadedScripts;

        mutable osgDB::ClassInterface _ci;
};


}

#endif
