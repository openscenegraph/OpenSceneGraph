TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	Export.cpp\
	Layer.cpp\
	Locator.cpp\
	TerrainNode.cpp\
	TerrainTechnique.cpp\

LIBS   += -losgTerrain -losg -losgIntrospection -losgDB -losgUtil -losgFX  $(GL_LIBS) $(OTHER_LIBS)

OPTF = -O

TARGET_BASENAME = osgTerrain
include $(TOPDIR)/Make/cygwin_wrapper_def
WRAPPER = $(WRAPPER_PREFIX)$(TARGET_BASENAME).$(PLUGIN_EXT)

include $(TOPDIR)/Make/makerules 
