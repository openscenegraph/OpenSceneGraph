TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 


CXXFILES = \
    ColorRange.cpp\
    ScalarBar.cpp\
    ScalarsToColors.cpp\
    BlinkSequence.cpp\
    LightPoint.cpp\
    LightPointDrawable.cpp\
    LightPointNode.cpp\
    Sector.cpp\
    SphereSegment.cpp\
    Version.cpp\

DEF     += -DOSGSIM_LIBRARY

LIBS  += -losg -losgUtil $(GL_LIBS) $(OTHER_LIBS)

TARGET_BASENAME = osgSim
LIB = $(LIB_PREFIX)$(TARGET_BASENAME).$(LIB_EXT)

include $(TOPDIR)/Make/makerules 
