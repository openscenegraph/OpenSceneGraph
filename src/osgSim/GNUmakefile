TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 


CXXFILES = \
    BlinkSequence.cpp\
    LightPoint.cpp\
    LightPointDrawable.cpp\
    LightPointNode.cpp\
    Sector.cpp\
    Version.cpp\

#    SphereSegment.cpp\

DEF     += -DOSGSIM_LIBRARY

LIBS  += -losg -losgUtil $(GL_LIBS) $(OTHER_LIBS)

TARGET_BASENAME = osgSim
LIB = $(LIB_PREFIX)$(TARGET_BASENAME).$(LIB_EXT)

include $(TOPDIR)/Make/makerules 
