TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CFILES =\
        video_out_rgb.c\

CXXFILES =\
        ReaderWriterXine.cpp\

INC += $(XINE_INCLUDES)

LIBS  += $(OSG_LIBS) $(XINE_LIBS) $(OTHER_LIBS)

ifeq ($(ARCH),64)
   CFLAGS += -fPIC
   CXXFLAGS += -fPIC
else
   CFLAGS += -DCOMPILE_ASSEMBLY
endif

TARGET_BASENAME = xine
include $(TOPDIR)/Make/cygwin_plugin_def
PLUGIN = $(PLUGIN_PREFIX)$(TARGET_BASENAME).$(PLUGIN_EXT)

include $(TOPDIR)/Make/makerules 
