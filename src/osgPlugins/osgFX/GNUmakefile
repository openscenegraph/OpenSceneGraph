TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs

CXXFILES =\
	IO_AnisotropicLighting.cpp\
	IO_BumpMapping.cpp\
	IO_Cartoon.cpp\
	IO_MultiTextureControl.cpp\
	IO_Scribe.cpp\
	IO_SpecularHighlights.cpp\
	IO_Effect.cpp\


LIBS   += -losgFX $(OSG_LIBS) $(OTHER_LIBS)

TARGET_BASENAME = osgFX
include $(TOPDIR)/Make/cygwin_plugin_def
PLUGIN = $(PLUGIN_PREFIX)$(TARGET_BASENAME).$(PLUGIN_EXT)

include $(TOPDIR)/Make/makerules
