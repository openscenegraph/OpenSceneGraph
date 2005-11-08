TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	AnisotropicLighting.cpp\
	BumpMapping.cpp\
	Cartoon.cpp\
	Effect.cpp\
	MultiTextureControl.cpp\
	Registry.cpp\
	Scribe.cpp\
	SpecularHighlights.cpp\
	Technique.cpp\
	Validator.cpp\

LIBS   += -losgFX -losg -losgIntrospection -losgUtil -losgDB  $(GL_LIBS) $(OTHER_LIBS)

OPTF = -O

TARGET_BASENAME = osgFX
include $(TOPDIR)/Make/cygwin_wrapper_def
WRAPPER = $(WRAPPER_PREFIX)$(TARGET_BASENAME).$(PLUGIN_EXT)

include $(TOPDIR)/Make/makerules 
