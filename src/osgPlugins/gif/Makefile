TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	ReaderWriterGIF.cpp\


INC   += -I/usr/local/include\
	 -I/usr/freeware/include\

LINKARGS += -L/usr/freeware/lib$(ARCH)

LIBS  += $(OSG_LIBS) $(GIF_LIBS) $(OTHER_LIBS)

TARGET_BASENAME = gif
include $(TOPDIR)/Make/cygwin_plugin_def
PLUGIN = $(PLUGIN_PREFIX)$(TARGET_BASENAME).$(PLUGIN_EXT)

include $(TOPDIR)/Make/makerules 
