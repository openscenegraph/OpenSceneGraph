SET(TARGET_NAME osgViewer )
SET(TARGET_SRC 
    View.cpp 
    ReaderWriterOsgViewer.cpp 
)
SET(TARGET_ADDED_LIBRARIES osgViewer )

#### end var setup  ###
SETUP_PLUGIN()
