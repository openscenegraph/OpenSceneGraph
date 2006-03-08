TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
        AncillaryRecords.cpp \
        AttrData.cpp \
        ControlRecords.cpp \
        DataInputStream.cpp \
        Document.cpp \
        GeometryRecords.cpp \
        LightPointRecords.cpp \
        PaletteRecords.cpp \
        Pools.cpp \
        PrimaryRecords.cpp \
        ReaderWriterATTR.cpp \
        ReaderWriterFLT.cpp \
        Record.cpp \
        RecordInputStream.cpp \
        Registry.cpp \
        ReservedRecords.cpp \
        RoadRecords.cpp \
        Vertex.cpp \
        VertexRecords.cpp \


INC += -I$(THISDIR)

LIBS  += -losgSim -losgText $(OSG_LIBS) $(OTHER_LIBS)

TARGET_BASENAME = OpenFlight
include $(TOPDIR)/Make/cygwin_plugin_def
PLUGIN = $(PLUGIN_PREFIX)$(TARGET_BASENAME).$(PLUGIN_EXT)

include $(TOPDIR)/Make/makerules 
