TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
        ReaderWriterJPEG.cpp\

ifneq ($(OS),HP-UX)
INC += -I/usr/local/include/
else
INC += $(JPEG_INCLUDE)
endif

LIBS  += $(OSG_LIBS) $(JPEG_LIBS) $(OTHER_LIBS)

TARGET_BASENAME = jpeg
include $(TOPDIR)/Make/cygwin_plugin_def
PLUGIN = $(PLUGIN_PREFIX)$(TARGET_BASENAME).$(PLUGIN_EXT)

include $(TOPDIR)/Make/makerules 
