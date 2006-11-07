TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 


CXXFILES = \
    OccluderGeometry.cpp\
    ShadowedScene.cpp\
    ShadowTechnique.cpp\
    ShadowMap.cpp\
    ShadowTexture.cpp\
    ShadowVolume.cpp\
    ParallelSplitShadowMap.cpp\
    Version.cpp\

DEF     += -DOSGSHADOW_LIBRARY

LIBS  +=  -losgUtil -losg $(GL_LIBS) $(OTHER_LIBS)

TARGET_BASENAME = osgShadow
LIB = $(LIB_PREFIX)$(TARGET_BASENAME).$(LIB_EXT)

include $(TOPDIR)/Make/makerules 
