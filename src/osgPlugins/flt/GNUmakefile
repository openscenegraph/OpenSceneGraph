TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
    BoundingVolumeRecords.cpp\
    LongIDRecord.cpp\
    ColorPaletteRecord.cpp\
    MaterialPaletteRecord.cpp\
    CommentRecord.cpp\
    ObjectRecord.cpp\
    ControlRecord.cpp\
    OldVertexRecords.cpp\
    OldMaterialPaletteRecord.cpp\
    DofRecord.cpp\
    ExtensionRecord.cpp\
    ExternalRecord.cpp\
    Record.cpp\
    FaceRecord.cpp\
    LocalVertexPoolRecord.cpp\
    MeshPrimitiveRecord.cpp\
    MeshRecord.cpp\
    RecordVisitor.cpp\
    RoadRecords.cpp\
    GeoSetBuilder.cpp\
    Registry.cpp\
    FltFile.cpp\
    GroupRecord.cpp\
    SwitchRecord.cpp\
    HeaderRecord.cpp\
    TexturePaletteRecord.cpp\
    Input.cpp\
    TransformationRecords.cpp\
    InstanceRecords.cpp\
    UnknownRecord.cpp\
    LightPointRecord.cpp\
    VertexPoolRecords.cpp\
    LightPointPaletteRecords.cpp\
    LightPointSystemRecord.cpp\
    LightSourcePaletteRecord.cpp\
    flt.cpp\
    LightSourceRecord.cpp\
    flt2osg.cpp\
    LodRecord.cpp\
    Pool.cpp\
    TextureMappingPaletteRecord.cpp\
    ReaderWriterFLT.cpp\
    ReaderWriterATTR.cpp\
    MultiTextureRecord.cpp\
    UVListRecord.cpp\
    BSPRecord.cpp\
    FindExternalModelVisitor.cpp\
#    PointLight.cpp\



INC += -I$(THISDIR)

LIBS  += -losgSim -losgText $(OSG_LIBS) $(OTHER_LIBS)

TARGET_BASENAME = flt
include $(TOPDIR)/Make/cygwin_plugin_def
PLUGIN = $(PLUGIN_PREFIX)$(TARGET_BASENAME).$(PLUGIN_EXT)

include $(TOPDIR)/Make/makerules 
