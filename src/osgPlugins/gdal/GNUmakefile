TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
        ReaderWriterGDAL.cpp\


INC += $(GDAL_INCLUDES)

LIBS  += $(OSG_LIBS) $(GDAL_LIBS) $(OTHER_LIBS)

TARGET_BASENAME = gdal

include $(TOPDIR)/Make/cygwin_plugin_def
PLUGIN = $(PLUGIN_PREFIX)$(TARGET_BASENAME).$(PLUGIN_EXT)

include $(TOPDIR)/Make/makerules 
