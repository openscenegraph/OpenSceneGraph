TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	BlinkSequence.cpp\
	ColorRange.cpp\
	DOFTransform.cpp\
	GeographicLocation.cpp\
	Impostor.cpp\
	ImpostorSprite.cpp\
	InsertImpostorsVisitor.cpp\
	LightPoint.cpp\
	LightPointNode.cpp\
	LightPointSystem.cpp\
	MultiSwitch.cpp\
	OpenFlightOptimizer.cpp\
	OverlayNode.cpp\
	ScalarBar.cpp\
	ScalarsToColors.cpp\
	Sector.cpp\
	SphereSegment.cpp\
	VisibilityGroup.cpp\

LIBS   += -losgSim -losg -losgIntrospection -losgUtil -losgText -losgDB  $(GL_LIBS) $(OTHER_LIBS)

OPTF = -O

TARGET_BASENAME = osgSim
include $(TOPDIR)/Make/cygwin_wrapper_def
WRAPPER = $(WRAPPER_PREFIX)$(TARGET_BASENAME).$(PLUGIN_EXT)

include $(TOPDIR)/Make/makerules 
