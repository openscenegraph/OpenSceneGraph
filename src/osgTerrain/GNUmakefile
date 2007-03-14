TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 


CXXFILES = \
    Locator.cpp\
    Layer.cpp\
    TerrainNode.cpp\
    TerrainTechnique.cpp\
    Version.cpp\

DEF     += -DOSGTERRAIN_LIBRARY

LIBS  +=  -losgDB -losgUtil -losg $(GL_LIBS) $(OTHER_LIBS)

TARGET_BASENAME = osgTerrain
LIB = $(LIB_PREFIX)$(TARGET_BASENAME).$(LIB_EXT)

include $(TOPDIR)/Make/makerules 
