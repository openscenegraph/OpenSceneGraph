#!gmake
include $(OSGHOME)/Make/makedefs

C++FILES = \
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
	


TARGET_BASENAME = osgdb_flt

LIB = 
LOADABLE = $(OSGHOME)/lib/osgPlugins/$(TARGET_BASENAME).$(DL_EXT)

TARGET_LOADER_FILES = osgPlugins/$(TARGET_BASENAME).$(DL_EXT)

LIBS    =  $(GL_LIBS) -losg -losgDB
MACOSXLIBS = -L$(OSGHOME)/lib -losg -losgDB -lstdc++

C++FLAGS += -I. -I$(OSGHOME)/include
LDFLAGS += -L$(OSGHOME)/lib

include $(OSGHOME)/Make/makerules

