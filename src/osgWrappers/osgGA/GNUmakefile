TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	AnimationPathManipulator.cpp\
	DriveManipulator.cpp\
	EventQueue.cpp\
	EventVisitor.cpp\
	FlightManipulator.cpp\
	GUIActionAdapter.cpp\
	GUIEventAdapter.cpp\
	GUIEventHandler.cpp\
	GUIEventHandlerVisitor.cpp\
	KeySwitchMatrixManipulator.cpp\
	MatrixManipulator.cpp\
	NodeTrackerManipulator.cpp\
	SetSceneViewVisitor.cpp\
	StateSetManipulator.cpp\
	TerrainManipulator.cpp\
	TrackballManipulator.cpp\
	UFOManipulator.cpp\

LIBS   += -losgGA -losg -losgIntrospection -losgUtil  $(GL_LIBS) $(OTHER_LIBS)

OPTF = -O

TARGET_BASENAME = osgGA
include $(TOPDIR)/Make/cygwin_wrapper_def
WRAPPER = $(WRAPPER_PREFIX)$(TARGET_BASENAME).$(PLUGIN_EXT)

include $(TOPDIR)/Make/makerules 
