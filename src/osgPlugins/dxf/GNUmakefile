TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	aci.cpp\
	dxfBlock.cpp\
	dxfEntity.cpp\
	dxfFile.cpp\
	dxfReader.cpp\
	dxfSection.cpp\
	dxfTable.cpp\
	ReaderWriterDXF.cpp\
	scene.cpp\


INC += -I$(THISDIR)

LIBS  += $(OSG_LIBS) $(OTHER_LIBS)

TARGET_BASENAME = dxf
include $(TOPDIR)/Make/cygwin_plugin_def
PLUGIN = $(PLUGIN_PREFIX)$(TARGET_BASENAME).$(PLUGIN_EXT)

include $(TOPDIR)/Make/makerules 
