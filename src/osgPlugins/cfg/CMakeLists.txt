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

#
# Android NDK r6, r6b and r7 ship with a broken gcc (armeabi) 
# see also 
# - http://gcc.gnu.org/bugzilla/show_bug.cgi?id=40836 
# - http://gcc.gnu.org/bugzilla/show_bug.cgi?id=37987
#
if (ANDROID)
    set_source_files_properties(ConfigParser.cpp 
        PROPERTY
        COMPILE_FLAGS
        "-O0")
endif()
# 

SET(TARGET_ADDED_LIBRARIES osgViewer)

#### end var setup  ###
SETUP_PLUGIN(cfg)
