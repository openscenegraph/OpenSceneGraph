TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
        Exception.cpp\
        Geode.cpp\
	ac3d.cpp\

LIBS  += $(OSG_LIBS) $(OTHER_LIBS)

TARGET_BASENAME = ac
include $(TOPDIR)/Make/cygwin_plugin_def
PLUGIN = $(PLUGIN_PREFIX)$(TARGET_BASENAME).$(PLUGIN_EXT)

include $(TOPDIR)/Make/makerules 
