TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	OSGA_Archive.cpp\
	ReaderWriterOSGA.cpp\


INC += -I$(THISDIR)

LIBS  += $(OSG_LIBS) $(OTHER_LIBS)

TARGET_BASENAME = osga
include $(TOPDIR)/Make/cygwin_plugin_def
PLUGIN = $(PLUGIN_PREFIX)$(TARGET_BASENAME).$(PLUGIN_EXT)

include $(TOPDIR)/Make/makerules 
