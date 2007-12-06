SET(TARGET_SRC 
  CameraConfig.cpp
  Camera.cpp
  ConfigLexer.cpp
  ConfigParser.cpp
  ReaderWriterCFG.cpp
  RenderSurface.cpp
  VisualChooser.cpp
)

SET(TARGET_H
  CameraConfig.h
  Camera.h
  ConfigParser.h
  RenderSurface.h
  VisualChooser.h
)

SET(TARGET_ADDED_LIBRARIES osgViewer)

#### end var setup  ###
SETUP_PLUGIN(cfg)
