TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 


CXXFILES = \
	AnimationPathManipulator.cpp\
	MatrixManipulator.cpp\
	DriveManipulator.cpp\
	FlightManipulator.cpp\
        GUIEventHandler.cpp\
        GUIEventHandlerVisitor.cpp\
        KeySwitchMatrixManipulator.cpp\
        SetSceneViewVisitor.cpp\
	StateSetManipulator.cpp\
	TerrainManipulator.cpp\
	TrackerManipulator.cpp\
	TrackballManipulator.cpp\
	Version.cpp\

DEF     += -DOSGGA_LIBRARY

LIBS  += -losgUtil -losg $(GL_LIBS) $(OTHER_LIBS)

TARGET_BASENAME = osgGA
LIB = $(LIB_PREFIX)$(TARGET_BASENAME).$(LIB_EXT)

include $(TOPDIR)/Make/makerules 
