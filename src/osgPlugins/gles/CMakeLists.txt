SET(TARGET_SRC
    ReaderWriterGLES.cpp
    OpenGLESGeometryOptimizer.cpp
    TriangleStripVisitor.cpp
    IndexMeshVisitor.cpp
    UnIndexMeshVisitor.cpp
    forsythtriangleorderoptimizer.cpp)

#### end var setup  ###
SET(TARGET_ADDED_LIBRARIES
    osgUtil)
SETUP_PLUGIN(gles)
