SET(TARGET_SRC
    ReaderWriterP3D.cpp
    ReaderWriterPaths.cpp
)

SET(TARGET_ADDED_LIBRARIES osgGA osgText osgVolume osgFX osgViewer osgPresentation)

#### end var setup  ###
SETUP_PLUGIN(p3d)
