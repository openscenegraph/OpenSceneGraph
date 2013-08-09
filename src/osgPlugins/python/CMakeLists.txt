SET(TARGET_H
    PythonScriptEngine.h
)
SET(TARGET_SRC
    PythonScriptEngine.cpp
    ReaderWriterPython.cpp
)

SET(TARGET_EXTERNAL_LIBRARIES ${TARGET_EXTERNAL_LIBRARIES} ${PYTHON_LIBRARY})
INCLUDE_DIRECTORIES(${PYTHON_INCLUDE_DIR} )

#### end var setup  ###
SETUP_PLUGIN(python)
