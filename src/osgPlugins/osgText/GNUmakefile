TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	IO_Text.cpp

LIBS   += -losgText $(OSG_LIBS) $(OTHER_LIBS)

TARGET_BASENAME = osgText
include $(TOPDIR)/Make/cygwin_plugin_def
PLUGIN = $(PLUGIN_PREFIX)$(TARGET_BASENAME).$(PLUGIN_EXT)

include $(TOPDIR)/Make/makerules 
