/* Copyright Robert Osfield, Licensed under the GPL
 *
 * Experimental base for refactor of Present3D
 *
*/

#include <osg/Geode>
#include <osg/Geometry>

#include <osgDB/ReadFile>
#include <osgDB/FileUtils>

#include <osgGA/TrackballManipulator>
#include <osgViewer/Viewer>

// hacky experiment with create lua, v8 and python to run scripts.

#ifdef USE_LUA

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

void runLua(const std::string& str)
{
    OSG_NOTICE<<std::endl<<"runLua("<<str<<")"<<std::endl;

    lua_State* lua = lua_open();

    luaL_openlibs(lua);

    std::string filePath = str;
    if(osgDB::fileExists(filePath))
    {
        if(luaL_dofile(lua, filePath.c_str()))
        {
            OSG_NOTICE << "LuaEngine::runFile - " << lua_tostring(lua, -1) << std::endl;
        }

    }
    else
    {
        OSG_NOTICE<<" parameter="<<filePath<<" not a valid file."<<std::endl;
    }

    lua_close(lua);
}

#endif

#ifdef USE_PYTHON

#include <Python.h>

void runPython(const std::string& str)
{
    OSG_NOTICE<<std::endl<<"runPython("<<str<<")"<<std::endl;

    Py_InitializeEx(0);

    PyObject* py_main = PyModule_GetDict(PyImport_AddModule("__main__"));

    std::string filePath = str;
    if(osgDB::fileExists(filePath))
    {
        FILE*     f = fopen(filePath.c_str(), "r");
        PyObject* r = PyRun_File(f, filePath.c_str(), Py_file_input, py_main, py_main);

        fclose(f);

        if(!r) {
            r = PyErr_Occurred();

            if(r) {
                // The following snippet lets us get the return code. That is: if the
                // script is stopped with sys.exit() or similar. We could use this
                // return code to do something sensible... later.
                if(PyErr_ExceptionMatches(PyExc_SystemExit)) {
                    PyObject* ty = 0;
                    PyObject* er = 0;
                    PyObject* tr = 0;

                    PyErr_Fetch(&ty, &er, &tr);

                    Py_DECREF(ty);
                    Py_DECREF(er);
                    Py_DECREF(er);
                }

                else {
                    PyErr_Print();
                    PyErr_Clear();
                }
            }
        }
    }

    Py_Finalize();
}

#endif


#ifdef USE_V8

#include <v8.h>

void runV8(const std::string& str)
{
    OSG_NOTICE<<std::endl<<"runV8("<<str<<")"<<std::endl;

    v8::Isolate* isolate = v8::Isolate::New();

    {
        v8::Locker locker(isolate);
        v8::Isolate::Scope isolate_scope(isolate);

        // Create a stack-allocated handle scope.
        v8::HandleScope handle_scope;

        v8::Handle<v8::ObjectTemplate> global = v8::ObjectTemplate::New();
        v8::Persistent<v8::Context> globalContext = v8::Context::New(NULL, global);
        v8::Persistent<v8::ObjectTemplate> globalTemplate = v8::Persistent<v8::ObjectTemplate>::New(global);

        std::string filePath = str;
        if(osgDB::fileExists(filePath))
        {
            std::string js_source;
            std::ifstream fin(filePath.c_str());
            if (fin)
            {
                // read file in the crude way.
                while(fin)
                {
                    int c = fin.get();
                    if (c>=0 && c<=255)
                    {
                        js_source.push_back(c);
                    }
                }

                // Create a nested handle scope
                v8::HandleScope local_handle_scope;

                // Enter the global context
                v8::Context::Scope context_scope(globalContext);

                // Create a string containing the JavaScript source code.
                v8::Handle<v8::String> source = v8::String::New(js_source.c_str());

                // Compile the source code.
                v8::Handle<v8::Script> script = v8::Script::Compile(source);

                // Run the script to get the result.
                v8::Handle<v8::Value> result = script->Run();

                // Convert the result to an ASCII string and print it.
                v8::String::AsciiValue ascii(result);
                printf("%s\n", *ascii);
            }
        }

    globalTemplate.Dispose();
    globalContext.Dispose();
  }
  isolate->Dispose();

}
#endif

int main(int argc, char** argv)
{
    osg::ArgumentParser arguments(&argc, argv);

    osgViewer::Viewer viewer(arguments);

    std::string str;

#ifdef USE_LUA
    while (arguments.read("--lua",str))
    {
        runLua(str);
    }
#endif

#ifdef USE_V8
    while (arguments.read("--js",str))
    {
        runV8(str);
    }
#endif

#ifdef USE_PYTHON
    while (arguments.read("--python",str))
    {
        runPython(str);
    }
#endif

    // create the model
    osg::ref_ptr<osg::Node> model = osgDB::readNodeFiles(arguments);
    if (!model) return 1;

    viewer.setSceneData( model.get() );
    return viewer.run();
}
