TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 


CXXFILES = \
    CoordinateSystem.cpp\
    DataSet.cpp\
    Terrain.cpp\
    Renderer.cpp\
    GeoMipMapRenderer.cpp\


DEF     += -DOSGTERRAIN_LIBRARY

LIBS  +=  -losgDB -losgUtil -losg -lgdal $(GL_LIBS) $(OTHER_LIBS)

TARGET_BASENAME = osgTerrain
LIB = $(LIB_PREFIX)$(TARGET_BASENAME).$(LIB_EXT)

include $(TOPDIR)/Make/makerules 
