TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	ReaderWriterQT.cpp\
	QTtexture.cpp\

LIBS  += $(QUICKTIME) $(OSG_LIBS) $(OTHER_LIBS)

TARGET_BASENAME = qt
PLUGIN = $(PLUGIN_PREFIX)$(TARGET_BASENAME).$(PLUGIN_EXT)

include $(TOPDIR)/Make/makerules 

