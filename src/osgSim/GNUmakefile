TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 


CXXFILES = \
	BlinkSequence.cpp\
	ColorRange.cpp\
	DOFTransform.cpp\
        Impostor.cpp\
	ImpostorSprite.cpp\
        InsertImpostorsVisitor.cpp\
	LightPoint.cpp\
	LightPointDrawable.cpp\
	LightPointSpriteDrawable.cpp\
	LightPointNode.cpp\
	MultiSwitch.cpp\
	OverlayNode.cpp\
	OpenFlightOptimizer.cpp\
	ScalarBar.cpp\
	ScalarsToColors.cpp\
	Sector.cpp\
	SphereSegment.cpp\
	Version.cpp\
	VisibilityGroup.cpp\


DEF     += -DOSGSIM_LIBRARY

LIBS  += -losgText -losgDB -losgUtil -losg $(GL_LIBS) $(OTHER_LIBS)

TARGET_BASENAME = osgSim
LIB = $(LIB_PREFIX)$(TARGET_BASENAME).$(LIB_EXT)

include $(TOPDIR)/Make/makerules 
