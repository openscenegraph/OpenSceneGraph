TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	AntiSquish.cpp\
	Command.cpp\
	CommandManager.cpp\
	Constraint.cpp\
	Dragger.cpp\
	Projector.cpp\
	RotateCylinderDragger.cpp\
	RotateSphereDragger.cpp\
	Scale1DDragger.cpp\
	Scale2DDragger.cpp\
	ScaleAxisDragger.cpp\
	Selection.cpp\
	TabBoxDragger.cpp\
	TabPlaneDragger.cpp\
	TabPlaneTrackballDragger.cpp\
	TrackballDragger.cpp\
	Translate1DDragger.cpp\
	Translate2DDragger.cpp\
	TranslateAxisDragger.cpp\
	TranslatePlaneDragger.cpp\

LIBS   += -losgManipulator -losg -losgIntrospection  $(GL_LIBS) $(OTHER_LIBS)

OPTF = -O

TARGET_BASENAME = osgManipulator
include $(TOPDIR)/Make/cygwin_wrapper_def
WRAPPER = $(WRAPPER_PREFIX)$(TARGET_BASENAME).$(PLUGIN_EXT)

include $(TOPDIR)/Make/makerules 
