TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
        ReaderWriterJPEG.cpp\

INC += -I/usr/local/include/

LIBS  += $(OSG_LIBS) $(OTHER_LIBS)

TARGET_BASENAME = jpeg
PLUGIN = $(PLUGIN_PREFIX)$(TARGET_BASENAME).$(PLUGIN_EXT)

include $(TOPDIR)/Make/makerules 
