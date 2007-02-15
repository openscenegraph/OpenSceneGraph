TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	ParallelSplitShadowMap.cpp\
	ShadowMap.cpp\
	ShadowTechnique.cpp\
	ShadowTexture.cpp\
	ShadowVolume.cpp\
	ShadowedScene.cpp

LIBS   += -losgShadow -losgUtil -losg $(OTHER_LIBS)

TARGET_BASENAME = osgShadow
include $(TOPDIR)/Make/cygwin_plugin_def
PLUGIN = $(PLUGIN_PREFIX)$(TARGET_BASENAME).$(PLUGIN_EXT)

include $(TOPDIR)/Make/makerules 
