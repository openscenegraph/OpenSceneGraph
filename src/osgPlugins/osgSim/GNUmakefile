TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	IO_LightPointNode.cpp\
	IO_LightPoint.cpp\
	IO_BlinkSequence.cpp\
	IO_Sector.cpp

LIBS   += -losgSim $(OSG_LIBS) $(OTHER_LIBS)

TARGET_BASENAME = osgSim
include $(TOPDIR)/Make/cygwin_plugin_def
PLUGIN = $(PLUGIN_PREFIX)$(TARGET_BASENAME).$(PLUGIN_EXT)

include $(TOPDIR)/Make/makerules 
