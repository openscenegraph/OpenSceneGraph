
SET(TARGET_SRC 
    View.cpp 
    Viewer.cpp 
    CompositeViewer.cpp 
    ReaderWriterOsgViewer.cpp 
)
SET(TARGET_ADDED_LIBRARIES osgViewer )

#### end var setup  ###
SETUP_PLUGIN(osgviewer)
