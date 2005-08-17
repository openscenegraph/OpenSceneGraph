TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CFILES =\
        video_out_rgb.c\

CXXFILES =\
        ReaderWriterXine.cpp\

INC += $(XINE_INCLUDES)

LIBS  += $(OSG_LIBS) $(XINE_LIBS) $(OTHER_LIBS)

CFLAGS += -fPIC
CXXFLAGS += -fPIC

TARGET_BASENAME = xine
include $(TOPDIR)/Make/cygwin_plugin_def
PLUGIN = $(PLUGIN_PREFIX)$(TARGET_BASENAME).$(PLUGIN_EXT)

include $(TOPDIR)/Make/makerules 
