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

#include "V8ScriptEngine.h"

using namespace v8;

V8ScriptEngine::V8ScriptEngine():
    osg::ScriptEngine("js"),
    _isolate(0)
{
    initialize();
}

V8ScriptEngine::V8ScriptEngine(const V8ScriptEngine& rhs, const osg::CopyOp&):
    osg::ScriptEngine("js"),
    _isolate(0)
{
    initialize();
}

V8ScriptEngine::~V8ScriptEngine()
{
    _isolate->Dispose();
    _isolate = 0;
}

void V8ScriptEngine::initialize()
{
    _isolate = v8::Isolate::New();

}

bool V8ScriptEngine::run(osg::Script* script, const std::string& entryPoint, osg::Parameters& inputParameters, osg::Parameters& outputParameters)
{
    if (!script || !_isolate) return false;

    v8::Locker locker(_isolate);
    v8::Isolate::Scope isolate_scope(_isolate);

    // Create a stack-allocated handle scope.
    v8::HandleScope handle_scope;

    v8::Handle<v8::ObjectTemplate> global = v8::ObjectTemplate::New();
    _globalContext = v8::Context::New(NULL, global);
    _globalTemplate = v8::Persistent<v8::ObjectTemplate>::New(global);

    {
        // Create a nested handle scope
        v8::HandleScope local_handle_scope;

        // Enter the global context
        v8::Context::Scope context_scope(_globalContext);

        // Create a string containing the JavaScript source code.
        v8::Handle<v8::String> source = v8::String::New(script->getScript().c_str());

        // Compile the source code.
        v8::Handle<v8::Script> v8_script = v8::Script::Compile(source);

        // Run the script to get the result.
        v8::Handle<v8::Value> result = v8_script->Run();

        // Convert the result to an ASCII string and print it.
        v8::String::AsciiValue ascii(result);
        printf("%s\n", *ascii);
    }

    _globalTemplate.Dispose();
    _globalContext.Dispose();

    return true;
}
