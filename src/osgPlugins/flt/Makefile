#!smake
include $(TOP)/Make/makedefs

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

TARGET_LIB_FILES = lib$(TARGET_BASENAME).$(SO_EXT)

LIB = $(TOP)/lib/osgPlugins/$(TARGET_BASENAME).$(SO_EXT)

TARGET_LOADER_FILES = osgPlugins/$(TARGET_BASENAME).$(SO_EXT)

LIBS    =  $(GL_LIBS) -losg -losgDB

C++FLAGS += -I. -I$(TOP)/include 

LDFLAGS += -L$(TOP)/lib

include $(TOP)/Make/makerules

