TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
        MpegImageStream.cpp\
        ReaderWriterMPEG.cpp\

MPEG_LIBS = -lmpeg3

LIBS  += $(OSG_LIBS) $(MPEG_LIBS) $(OTHER_LIBS)

TARGET_BASENAME = mpeg
include $(TOPDIR)/Make/cygwin_plugin_def
PLUGIN = $(PLUGIN_PREFIX)$(TARGET_BASENAME).$(PLUGIN_EXT)

include $(TOPDIR)/Make/makerules 
