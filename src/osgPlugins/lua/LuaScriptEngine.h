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
#include <osgDB/PropertyInterface>

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
        virtual bool run(osg::Script* script, const std::string& entryPoint, Parameters& inputParameters, Parameters& outputParameters);

        /** get the lua_State object.*/
        lua_State* getLuaState() { return _lua; }

        int pushPropertyToStack(osg::Object* object, const std::string& propertyName) const;
        int setPropertyFromStack(osg::Object* object, const std::string& propertyName) const;

        bool loadScript(osg::Script* script);

        osgDB::BaseSerializer::Type getType() const;

        bool getfields(const char* f1, const char* f2, int type) const;
        bool getfields(const char* f1, const char* f2, const char* f3, int type) const;
        bool getfields(const char* f1, const char* f2, const char* f3, const char* f4, int type) const;
        bool getelements(int numElements, int type) const;

        bool getvec2() const;
        bool getvec3() const;
        bool getvec4() const;
        bool getmatrix() const;

        bool getValue(osg::Vec2f& value) const;
        bool getValue(osg::Vec3f& value) const;
        bool getValue(osg::Vec4f& value) const;
        bool getValue(osg::Matrixf& value) const;

        bool getValue(osg::Vec2d& value) const;
        bool getValue(osg::Vec3d& value) const;
        bool getValue(osg::Vec4d& value) const;
        bool getValue(osg::Quat& value) const;
        bool getValue(osg::Plane& value) const;
        bool getValue(osg::Matrixd& value) const;

        void pushValue(const osg::Vec2f& value) const;
        void pushValue(const osg::Vec3f& value) const;
        void pushValue(const osg::Vec4f& value) const;
        void pushValue(const osg::Matrixf& value) const;

        void pushValue(const osg::Vec2d& value) const;
        void pushValue(const osg::Vec3d& value) const;
        void pushValue(const osg::Vec4d& value) const;
        void pushValue(const osg::Quat& value) const;
        void pushValue(const osg::Plane& value) const;
        void pushValue(const osg::Matrixd& value) const;

        bool pushParameter(osg::Object* object);
        bool popParameter(osg::Object* object);


        void createAndPushObject(const std::string& compoundName) const;
        void pushObject(osg::Object* object) const;

    protected:

        void initialize();

        virtual ~LuaScriptEngine();


        lua_State* _lua;

        typedef std::set< osg::ref_ptr<osg::Script> > ScriptSet;
        ScriptSet _loadedScripts;

        mutable osgDB::PropertyInterface _pi;
};


}

#endif
