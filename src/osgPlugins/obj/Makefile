TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	glm.cpp\
	ReaderWriterOBJ.cpp\


INC += -I$(THISDIR)

LIBS  += $(OSG_LIBS) $(OTHER_LIBS)

TARGET_BASENAME = obj
PLUGIN = $(PLUGIN_PREFIX)$(TARGET_BASENAME).$(PLUGIN_EXT)

include $(TOPDIR)/Make/makerules 
