TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 


CXXFILES = \
    DataSet.cpp\
    HeightFieldNode.cpp\
    HeightFieldRenderer.cpp\
    Version.cpp\


DEF     += -DOSGTERRAIN_LIBRARY

LIBS  +=  -losgDB -losgUtil -losg -lgdal $(GL_LIBS) $(OTHER_LIBS)

TARGET_BASENAME = osgTerrain
LIB = $(LIB_PREFIX)$(TARGET_BASENAME).$(LIB_EXT)

include $(TOPDIR)/Make/makerules 
