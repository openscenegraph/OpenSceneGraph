#!smake
include $(TOP)/Make/makedefs

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


#LIBS    = -L$(TOP)/lib -losg -lGLU -lGL -lm -ldl
LIBS    = -L$(TOP)/lib -losg $(GL_LIBS) $(DYNAMICLIBRARYLIB)

LIB	= $(TOP)/lib/lib$(TARGET_BASENAME).$(SO_EXT)
#LIB	= $(TOP)/lib/lib$(TARGET_BASENAME).a

C++FLAGS += -I $(TOP)/include -DOSGDB_LIBRARY

#LDFLAGS += -Wl,--out-implib,./../lib/lib$(TARGET_BASENAME).a

include $(TOP)/Make/makerules

