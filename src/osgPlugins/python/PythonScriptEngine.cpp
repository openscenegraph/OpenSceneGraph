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

#include "PythonScriptEngine.h"

using namespace python;

PythonScriptEngine::PythonScriptEngine():
    osg::ScriptEngine("python"),
    _py_main(0)
{
    initialize();
}

PythonScriptEngine::PythonScriptEngine(const PythonScriptEngine& rhs, const osg::CopyOp&):
    osg::ScriptEngine("python"),
    _py_main(0)
{
    initialize();
}

PythonScriptEngine::~PythonScriptEngine()
{
    Py_Finalize();
}

void PythonScriptEngine::initialize()
{
    Py_InitializeEx(0);

    _py_main = PyModule_GetDict(PyImport_AddModule("__main__"));
}

bool PythonScriptEngine::run(osg::Script* script, const std::string& entryPoint, osg::Parameters& inputParameters, osg::Parameters& outputParameters)
{
    if (!script || !_py_main) return false;

    PyObject* r = PyRun_String(script->getScript().c_str(), Py_file_input, _py_main, _py_main);

    if(!r) {
        r = PyErr_Occurred();

        if(r) {
            PyErr_Print();
            PyErr_Clear();
        }
    }

    return true;
}
