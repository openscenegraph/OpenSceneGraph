TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	ReaderWriterTIFF.cpp\

LIBS  += $(OSG_LIBS) $(TIFF_LIB) $(OTHER_LIBS)

ifeq ($(OS),HP-UX)
INC     += $(TIFF_INCLUDE)
else
INC     += -I/usr/local/include
LDFLAGS += -L/usr/local/lib
endif

TARGET_BASENAME = tiff
include $(TOPDIR)/Make/cygwin_plugin_def
PLUGIN = $(PLUGIN_PREFIX)$(TARGET_BASENAME).$(PLUGIN_EXT)

include $(TOPDIR)/Make/makerules 
