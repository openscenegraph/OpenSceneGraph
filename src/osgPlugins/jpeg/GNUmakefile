TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
        ReaderWriterJPEG.cpp\

INC += $(JPEG_INCLUDE)

LIBS  += $(OSG_LIBS) $(JPEG_LIBS) $(OTHER_LIBS)

TARGET_BASENAME = jpeg
include $(TOPDIR)/Make/cygwin_plugin_def
PLUGIN = $(PLUGIN_PREFIX)$(TARGET_BASENAME).$(PLUGIN_EXT)

include $(TOPDIR)/Make/makerules 
