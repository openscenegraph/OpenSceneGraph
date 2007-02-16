TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	FadeText.cpp\
	Font.cpp\
	String.cpp\

LIBS   += -losgText -losg -losgIntrospection -losgDB  $(GL_LIBS) $(OTHER_LIBS)

OPTF = -O

TARGET_BASENAME = osgText
include $(TOPDIR)/Make/cygwin_wrapper_def
WRAPPER = $(WRAPPER_PREFIX)$(TARGET_BASENAME).$(PLUGIN_EXT)

include $(TOPDIR)/Make/makerules 
