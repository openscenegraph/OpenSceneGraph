TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
        ReaderWriterPNG.cpp\

LIBS  += $(OSG_LIBS) $(PNG_LIBS) $(OTHER_LIBS)
INC  += -I/usr/local/include\
	-I/usr/freeware/include\

LINKARGS += -L/usr/freeware/lib$(ARCH)

TARGET_BASENAME = png
PLUGIN = $(PLUGIN_PREFIX)$(TARGET_BASENAME).$(PLUGIN_EXT)

include $(TOPDIR)/Make/makerules 
