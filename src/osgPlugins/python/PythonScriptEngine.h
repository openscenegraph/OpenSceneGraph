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

#ifndef PYTHONSCRIPTENGINE_H
#define PYTHONSCRIPTENGINE_H

#include <osg/ScriptEngine>

#include <Python.h>

namespace python
{

class PythonScriptEngine : public osg::ScriptEngine
{
    public:
        PythonScriptEngine();
        PythonScriptEngine(const PythonScriptEngine& rhs, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);

        META_Object(lua, PythonScriptEngine)

        virtual const std::string& getLanguage() const { return _language; }

        /** run a Script.*/
        virtual bool run(osg::Script* script, const std::string& entryPoint, osg::Parameters& inputParameters, osg::Parameters& outputParameters);

        /** get the Python main object.*/
        PyObject* getPythonMain() { return _py_main; }

    protected:

        void initialize();

        virtual ~PythonScriptEngine();

        PyObject* _py_main;

};

}

#endif
