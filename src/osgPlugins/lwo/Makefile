TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	lw.cpp\
	ReaderWriterLWO.cpp\


INC += -I$(THISDIR)

LIBS  += $(OSG_LIBS) $(OTHER_LIBS)

TARGET_BASENAME = lwo
PLUGIN = $(PLUGIN_PREFIX)$(TARGET_BASENAME).$(PLUGIN_EXT)

include $(TOPDIR)/Make/makerules 
