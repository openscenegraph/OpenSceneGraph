SET(TARGET_SRC 
  AnimtkViewer.cpp
  AnimtkViewerKeyHandler.cpp
  AnimtkViewerGUI.cpp
  )
SET(TARGET_ADDED_LIBRARIES osgAnimation osgWidget)
SETUP_EXAMPLE(osganimationviewer)
