TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	geoActions.cpp\
	ClipRegion.cpp\
	ReaderWriterGEO.cpp


LIBS  += -losgText -losgSim $(OSG_LIBS) $(OTHER_LIBS)


TARGET_BASENAME = geo
include $(TOPDIR)/Make/cygwin_plugin_def
PLUGIN = $(PLUGIN_PREFIX)$(TARGET_BASENAME).$(PLUGIN_EXT)

include $(TOPDIR)/Make/makerules 
