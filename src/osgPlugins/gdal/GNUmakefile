TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
        ReaderWriterGDAL.cpp\

ifneq ("$(GDAL_INCLUDES)","")
INC += $(GDAL_INCLUDES)
endif

ifneq ("$(GDAL_LIB_DIR)","")
LDFLAGS += -L$(GDAL_LIB_DIR)
endif

LIBS  += $(OSG_LIBS) -lgdal $(OTHER_LIBS)

TARGET_BASENAME = gdal

include $(TOPDIR)/Make/cygwin_plugin_def
PLUGIN = $(PLUGIN_PREFIX)$(TARGET_BASENAME).$(PLUGIN_EXT)

include $(TOPDIR)/Make/makerules 
