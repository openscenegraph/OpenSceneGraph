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
        int getDataFromStack(SerializerScratchPad* ssp, osgDB::BaseSerializer::Type type, int pos) const;

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

        bool getValue(int pos, osg::Vec2f& value) const;
        bool getValue(int pos, osg::Vec3f& value) const;
        bool getValue(int pos, osg::Vec4f& value) const;

        bool getValue(int pos, osg::Vec2d& value) const;
        bool getValue(int pos, osg::Vec3d& value) const;
        bool getValue(int pos, osg::Vec4d& value) const;
        bool getValue(int pos, osg::Quat& value) const;
        bool getValue(int pos, osg::Plane& value) const;

        bool getValue(int pos, osg::Matrixf& value) const;
        bool getValue(int pos, osg::Matrixd& value) const;

        bool getValue(int pos, osg::BoundingBoxf& value) const;
        bool getValue(int pos, osg::BoundingBoxd& value) const;

        bool getValue(int pos, osg::BoundingSpheref& value) const;
        bool getValue(int pos, osg::BoundingSphered& value) const;

        void pushValue(osgDB::BaseSerializer::Type type, const void* ptr) const;

        void pushValue(const osg::Vec2f& value) const;
        void pushValue(const osg::Vec3f& value) const;
        void pushValue(const osg::Vec4f& value) const;

        void pushValue(const osg::Vec2d& value) const;
        void pushValue(const osg::Vec3d& value) const;
        void pushValue(const osg::Vec4d& value) const;
        void pushValue(const osg::Quat& value) const;
        void pushValue(const osg::Plane& value) const;

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
