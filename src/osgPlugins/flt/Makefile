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
	RecordVisitor.cpp\
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
	LightSourcePaletteRecord.cpp\
	flt.cpp\
	LightSourceRecord.cpp\
	flt2osg.cpp\
	LodRecord.cpp\
	Pool.cpp\
	TextureMappingPaletteRecord.cpp\
	ReaderWriterFLT.cpp\
	ReaderWriterATTR.cpp\
#	PointLight.cpp\



INC += -I$(THISDIR)

LIBS  += $(OSG_LIBS) $(OTHER_LIBS)

PLUGIN = osgdb_flt.$(PLUGIN_EXT)

include $(TOPDIR)/Make/makerules 
