TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 


CXXFILES = \
    ShadowedScene.cpp\
    ShadowTechnique.cpp\
    ShadowMap.cpp\
    ShadowTexture.cpp\
    ShadowVolume.cpp\
    ParallelSplitShadowMap.cpp\
    Version.cpp\

DEF     += -DOSGTEXT_LIBRARY

LIBS  +=  -losgDB -losg $(GL_LIBS) $(OTHER_LIBS)

TARGET_BASENAME = osgText
LIB = $(LIB_PREFIX)$(TARGET_BASENAME).$(LIB_EXT)

include $(TOPDIR)/Make/makerules 
