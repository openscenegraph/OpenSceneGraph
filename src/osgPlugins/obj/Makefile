TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	glm.cpp\
	ReaderWriterOBJ.cpp\


INC += -I$(THISDIR)

LIBS  += $(OSG_LIBS) $(OTHER_LIBS)

PLUGIN = osgdb_obj.$(PLUGIN_EXT)

include $(TOPDIR)/Make/makerules 
