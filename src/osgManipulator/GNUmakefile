TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 


CXXFILES = \
	AntiSquish.cpp \
	Command.cpp \
	CommandManager.cpp \
	Constraint.cpp \
	Dragger.cpp \
	Projector.cpp \
	RotateCylinderDragger.cpp \
	RotateSphereDragger.cpp \
	Scale1DDragger.cpp \
	Scale2DDragger.cpp \
	ScaleAxisDragger.cpp \
	Selection.cpp \
	TabBoxDragger.cpp \
	TabPlaneDragger.cpp \
	TabPlaneTrackballDragger.cpp \
	TrackballDragger.cpp \
	Translate1DDragger.cpp \
	Translate2DDragger.cpp \
	TranslateAxisDragger.cpp \
	TranslatePlaneDragger.cpp

DEF     += -DOSGMANIPULATOR_LIBRARY

INC   += 
LIBS  += -losgGA -losgUtil -losg $(GL_LIBS) $(OTHER_LIBS)

TARGET_BASENAME = osgManipulator

LIB = $(LIB_PREFIX)$(TARGET_BASENAME).$(LIB_EXT)

include $(TOPDIR)/Make/makerules 

