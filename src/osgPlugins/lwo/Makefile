TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	lw.cpp\
	ReaderWriterLWO.cpp\


INC += -I$(THISDIR)

LIBS  += $(OSG_LIBS) $(OTHER_LIBS)

PLUGIN = osgdb_lwo.$(PLUGIN_EXT)

include $(TOPDIR)/Make/makerules 
