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

#ifndef V8SCRIPTENGINE_H
#define V8SCRIPTENGINE_H

#include <osg/ScriptEngine>

#include <v8.h>

namespace v8
{

class V8ScriptEngine : public osg::ScriptEngine
{
    public:
        V8ScriptEngine();
        V8ScriptEngine(const V8ScriptEngine& rhs, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);

        META_Object(v8, V8ScriptEngine)

        virtual const std::string& getLanguage() const { return _language; }

        /** run a Script.*/
        virtual bool run(osg::Script* script, const std::string& entryPoint, osg::Parameters& inputParameters, osg::Parameters& outputParameters);

        v8::Isolate* getIsolate() { return _isolate; }

    protected:

        void initialize();

        virtual ~V8ScriptEngine();

        v8::Isolate* _isolate;
        v8::Persistent<v8::Context> _globalContext;
        v8::Persistent<v8::ObjectTemplate> _globalTemplate;
};


}

#endif
