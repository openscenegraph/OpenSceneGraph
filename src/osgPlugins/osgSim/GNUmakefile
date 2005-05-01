TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	IO_DOFTransform.cpp\
	IO_LightPointNode.cpp\
	IO_LightPoint.cpp\
	IO_BlinkSequence.cpp\
	IO_MultiSwitch.cpp\
	IO_Sector.cpp\
	IO_VisibilityGroup.cpp\
        IO_Impostor.cpp\


LIBS   += -losgSim -losgText $(OSG_LIBS) $(OTHER_LIBS)

TARGET_BASENAME = osgSim
include $(TOPDIR)/Make/cygwin_plugin_def
PLUGIN = $(PLUGIN_PREFIX)$(TARGET_BASENAME).$(PLUGIN_EXT)

include $(TOPDIR)/Make/makerules 
