TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES = ReaderWriterQT.cpp
CFILES = QTtexture.c

LIBS  += $(QUICKTIME) $(OSG_LIBS) $(OTHER_LIBS)

TARGET_BASENAME = qt
include $(TOPDIR)/Make/cygwin_plugin_def
PLUGIN = $(PLUGIN_PREFIX)$(TARGET_BASENAME).$(PLUGIN_EXT)

include $(TOPDIR)/Make/makerules 

