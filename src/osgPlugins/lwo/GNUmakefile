TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	old_lw.cpp\
        old_Lwo2.cpp\
        old_Lwo2Layer.cpp\
        Block.cpp\
        Clip.cpp\
        Converter.cpp\
        Object.cpp\
        Polygon.cpp\
        Surface.cpp\
        Tessellator.cpp\
        Unit.cpp\
        VertexMap.cpp\
	ReaderWriterLWO.cpp\


INC += -I$(THISDIR)

LIBS  += -losgFX $(OSG_LIBS) $(OTHER_LIBS) $(GL_LIBS)

TARGET_BASENAME = lwo
include $(TOPDIR)/Make/cygwin_plugin_def
PLUGIN = $(PLUGIN_PREFIX)$(TARGET_BASENAME).$(PLUGIN_EXT)

include $(TOPDIR)/Make/makerules 
