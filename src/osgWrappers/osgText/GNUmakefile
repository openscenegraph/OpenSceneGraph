TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	Export.cpp\
	Font.cpp\
	String.cpp\
	Text.cpp\

LIBS   += -losgText -losg -losgIntrospection -losgDB  $(GL_LIBS) $(OTHER_LIBS)

OPTF = -O

TARGET_BASENAME = osgText
include $(TOPDIR)/Make/cygwin_wrapper_def
WRAPPER = $(WRAPPER_PREFIX)$(TARGET_BASENAME).$(PLUGIN_EXT)

include $(TOPDIR)/Make/makerules 
