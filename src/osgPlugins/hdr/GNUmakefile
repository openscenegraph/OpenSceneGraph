TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
        hdrloader.cpp\
	ReaderWriterHDR.cpp\

LIBS  += $(OSG_LIBS) $(OTHER_LIBS)

TARGET_BASENAME = hdr
include $(TOPDIR)/Make/cygwin_plugin_def
PLUGIN = $(PLUGIN_PREFIX)$(TARGET_BASENAME).$(PLUGIN_EXT)

include $(TOPDIR)/Make/makerules 
