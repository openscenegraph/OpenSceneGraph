// -*-c++-*- osgWidget - Code by: Jeremy Moles (cubicool) 2007-2008

// Python.h needs to be included before anything else.
#ifdef OSGWIDGET_USEPYTHON
#include <Python.h>
#endif

#include <osgDB/FileUtils>
#include <osgWidget/Python>
#include <osgWidget/Box>
#include <osgWidget/WindowManager>

namespace osgWidget {

// Our Python library.
namespace py {
#ifdef OSGWIDGET_USEPYTHON

// TODO: Until I can find a way to move data around inside of the context, this will
// have to do. I don't really like it, but I've got no choice.
static PyObject* G_ERR = 0;

PyObject* newWindow(PyObject* self, PyObject* args) {
    PyObject*   buffer = 0;
    const char* name   = 0;
    int         width  = 0;
    int         height = 0;

    /*
    if(PyArg_ParseTuple(args, "sO!ii", &name, &PyBuffer_Type, &buffer, &width, &height)) {
        const void* buf = 0;
        int         len = 0;

        if(!PyObject_AsReadBuffer(buffer, &buf, &len)) {
            // if(Database::instance().add(name, buf, width, height))
            return Py_BuildValue("i", len);

            // else PyErr_SetString(G_ERR, "Couldn't add image to database.");
        }

        else PyErr_SetString(G_ERR, "Couldn't read buffer data.");

        return 0;
    }
    */

    PyErr_SetString(G_ERR, "still testing...");

    return 0;
}

static PyMethodDef methods[] = {
    {
        "newWindow", newWindow, METH_VARARGS,
        "docstring"
    },
    { 0, 0, 0, 0 }
};

#endif
}

// A helper function for all those cases where we need to inform the user that there isn't
// a LUA engine available.
bool noPythonFail(const std::string& err) {
    warn() << err << "; Python not compiled in library." << std::endl;

    return false;
}

// Our "private", internal data.
struct PythonEngineData {
#ifdef OSGWIDGET_USEPYTHON
    PythonEngineData():
    mod  (0),
    err  (0),
    main (0) {
    }

    bool valid() const {
        return mod && err && main;
    }

    PyObject* mod;
    PyObject* err;
    PyObject* main;
#endif
};

PythonEngine::PythonEngine(WindowManager* wm):
_wm(wm) {
#ifdef OSGWIDGET_USEPYTHON
    _data = new PythonEngineData();

#else
    _data = 0;
#endif
}

bool PythonEngine::initialize() {
#ifdef OSGWIDGET_USEPYTHON
    Py_InitializeEx(0);

    if(!_data->valid()) {
        _data->mod  = Py_InitModule3("osgwidget", py::methods, "main docstring");
        _data->err  = PyErr_NewException((char*)("osgwidget.error"), 0, 0);
        _data->main = PyModule_GetDict(PyImport_AddModule("__main__"));

        Py_INCREF(_data->err);

        // TODO: ...sigh...
        py::G_ERR = _data->err;

        PyModule_AddObject(_data->mod, "error", _data->err);
    }

    return true;

#else
    return noPythonFail("Can't initialize the PythonEngine");
#endif
}

bool PythonEngine::close() {
#ifdef OSGWIDGET_USEPYTHON
    if(_data->valid()) {
        Py_DECREF(_data->err);

        Py_Finalize();
    }

    delete _data;

    return true;

#else
    return noPythonFail("Can't close the PythonEngine");
#endif
}

bool PythonEngine::eval(const std::string& code) {
#ifdef OSGWIDGET_USEPYTHON
    PyObject* r = PyRun_String(code.c_str(), Py_file_input, _data->main, _data->main);

    if(!r) {
        r = PyErr_Occurred();

        if(r) {
            PyErr_Print();
            PyErr_Clear();
        }

        return false;
    }

    return true;

#else
    return noPythonFail("Can't evaluate code in PythonEngine");
#endif
}

bool PythonEngine::runFile(const std::string& filePath) {
#ifdef OSGWIDGET_USEPYTHON
    if(!osgDB::fileExists(filePath)) {
        warn()
            << "Couldn't find file \"" << filePath << "\" for PythonEngine."
            << std::endl
        ;

        return false;
    }

    FILE*     f = fopen(filePath.c_str(), "r");
    PyObject* r = PyRun_File(f, filePath.c_str(), Py_file_input, _data->main, _data->main);

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

        return false;
    }

    return true;

#else
    return noPythonFail("Can't evaluate code in PythonEngine");
#endif
}

}
