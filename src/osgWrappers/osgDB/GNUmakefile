TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	FileUtils.cpp\
	ReentrantMutex.cpp\
	Registry.cpp\

LIBS   += -losgDB -losg -losgIntrospection  $(GL_LIBS) $(OTHER_LIBS)

OPTF = -O

TARGET_BASENAME = osgDB
include $(TOPDIR)/Make/cygwin_wrapper_def
WRAPPER = $(WRAPPER_PREFIX)$(TARGET_BASENAME).$(PLUGIN_EXT)

include $(TOPDIR)/Make/makerules 
