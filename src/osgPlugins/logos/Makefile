TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	ReaderWriterLOGO.cpp\

LIBS  += $(OSG_LIBS) $(GL_LIBS) $(OTHER_LIBS)

TARGET_BASENAME = logo
include $(TOPDIR)/Make/cygwin_plugin_def
PLUGIN = $(PLUGIN_PREFIX)$(TARGET_BASENAME).$(PLUGIN_EXT)

include $(TOPDIR)/Make/makerules 
