TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 


CXXFILES = \
    BlinkSequence.cpp\
    ColorRange.cpp\
    DOFTransform.cpp\
    LightPoint.cpp\
    LightPointDrawable.cpp\
    LightPointNode.cpp\
    ScalarBar.cpp\
    ScalarsToColors.cpp\
    Sector.cpp\
    SphereSegment.cpp\
    MultiSwitch.cpp\
    Version.cpp\

DEF     += -DOSGSIM_LIBRARY

LIBS  += -losgText -losgDB -losgUtil -losg $(GL_LIBS) $(OTHER_LIBS)

TARGET_BASENAME = osgSim
LIB = $(LIB_PREFIX)$(TARGET_BASENAME).$(LIB_EXT)

include $(TOPDIR)/Make/makerules 
