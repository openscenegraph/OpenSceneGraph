TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	ReaderWriterDX.cpp \
        DXWriter.cpp \
        AreaGeoSetTriangulator.cpp \
        StateSetStr.cpp

LIBS  += $(OSG_LIBS) $(OTHER_LIBS)

TARGET_BASENAME = dx
PLUGIN = $(PLUGIN_PREFIX)$(TARGET_BASENAME).$(PLUGIN_EXT)

include $(TOPDIR)/Make/makerules 
