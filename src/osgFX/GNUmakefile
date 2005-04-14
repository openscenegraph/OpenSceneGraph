TOPDIR = ../..
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

LIBS += -losg -losgDB -losgUtil $(GL_LIBS) $(OTHER_LIBS) $(DYNAMICLIBRARYLIB)

DEF  += -DOSGFX_LIBRARY

TARGET_BASENAME = osgFX
LIB = $(LIB_PREFIX)$(TARGET_BASENAME).$(LIB_EXT)

include $(TOPDIR)/Make/makerules
