TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	ReaderWriterMD2.cpp\


INC += -I$(THISDIR)

LIBS  += $(OSG_LIBS) $(OTHER_LIBS)

TARGET_BASENAME = md2
include $(TOPDIR)/Make/cygwin_plugin_def
PLUGIN = $(PLUGIN_PREFIX)$(TARGET_BASENAME).$(PLUGIN_EXT)

include $(TOPDIR)/Make/makerules 
