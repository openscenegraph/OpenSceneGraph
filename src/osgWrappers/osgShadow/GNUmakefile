TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	Export.cpp\
	OccluderGeometry.cpp\
	ParallelSplitShadowMap.cpp\
	ShadowMap.cpp\
	ShadowTechnique.cpp\
	ShadowTexture.cpp\
	ShadowVolume.cpp\
	ShadowedScene.cpp\

LIBS   += -losgShadow -losg -losgIntrospection  $(GL_LIBS) $(OTHER_LIBS)

OPTF = -O

TARGET_BASENAME = osgShadow
include $(TOPDIR)/Make/cygwin_wrapper_def
WRAPPER = $(WRAPPER_PREFIX)$(TARGET_BASENAME).$(PLUGIN_EXT)

include $(TOPDIR)/Make/makerules 
