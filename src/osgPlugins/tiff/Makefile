TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	ReaderWriterTIFF.cpp\

LIBS  += $(TIFF_LIB) $(OSG_LIBS) $(OTHER_LIBS)

INC     += -I/usr/local/include
LDFLAGS += -L/usr/local/lib

TARGET_BASENAME = tiff
PLUGIN = $(PLUGIN_PREFIX)$(TARGET_BASENAME).$(PLUGIN_EXT)

include $(TOPDIR)/Make/makerules 
