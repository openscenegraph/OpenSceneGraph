TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 


CXXFILES = \
    DataSet.cpp\
    HeightFieldNode.cpp\
    HeightFieldRenderer.cpp\
    Version.cpp\

ifneq ("$(GDAL_INCLUDES)","")
INC += $(GDAL_INCLUDES)
endif

ifneq ("$(GDAL_LIB_DIR)","")
LDFLAGS += -L$(GDAL_LIB_DIR)
endif

DEF     += -DOSGTERRAIN_LIBRARY

LIBS  +=  -losgDB -losgUtil -losg -lgdal $(GL_LIBS) $(OTHER_LIBS)

TARGET_BASENAME = osgTerrain
LIB = $(LIB_PREFIX)$(TARGET_BASENAME).$(LIB_EXT)

include $(TOPDIR)/Make/makerules 
