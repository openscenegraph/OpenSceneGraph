TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
        ReaderWriterPNG.cpp\

LIBS  += $(OSG_LIBS) $(OTHER_LIBS)
INC  += -I/usr/local/include\
	-I/usr/freeware/include\

LINKARGS += -L/usr/freeware/lib$(ARCH)

PLUGIN = osgdb_png.$(PLUGIN_EXT)

include $(TOPDIR)/Make/makerules 
