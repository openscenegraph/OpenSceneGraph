#!smake
include $(OSGHOME)/Make/makedefs

C++FILES = \
	DynamicLibrary.cpp\
	FileNameUtils.cpp\
	FileUtils.cpp\
	ReadFile.cpp\
	Registry.cpp\
	Version.cpp\
	WriteFile.cpp\
	Input.cpp\
	Output.cpp\
	Field.cpp\
	FieldReader.cpp\
	FieldReaderIterator.cpp\
	DotOsgWrapper.cpp\
        

TARGET_BASENAME = osgDB

TARGET_LIB_FILES = lib$(TARGET_BASENAME).$(SO_EXT)

TARGET_INCLUDE_FILES = \
	osgDB/Registry\
	osgDB/DynamicLibrary\
	osgDB/Export\
	osgDB/FileNameUtils\
	osgDB/FileUtils\
	osgDB/ReaderWriter\
	osgDB/ReadFile\
	osgDB/Version\
	osgDB/WriteFile\
	osgDB/Input\
	osgDB/Output\
	osgDB/Field\
	osgDB/FieldReader\
	osgDB/FieldReaderIterator\
	osgDB/DotOsgWrapper\


#LIBS    = -L$(OSGHOME)/lib -losg -lGLU -lGL -lm -ldl
LIBS    = -L$(OSGHOME)/lib -losg $(GL_LIBS) $(DYNAMICLIBRARYLIB)

LIB	= $(OSGHOME)/lib/lib$(TARGET_BASENAME).$(SO_EXT)
#LIB	= $(OSGHOME)/lib/lib$(TARGET_BASENAME).a

C++FLAGS += -I $(OSGHOME)/include -DOSGDB_LIBRARY

#LDFLAGS += -Wl,--out-implib,./../lib/lib$(TARGET_BASENAME).a

include $(OSGHOME)/Make/makerules

