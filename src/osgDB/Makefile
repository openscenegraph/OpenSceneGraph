#!smake
include ../../Make/makedefs

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

TARGET_LIB_FILES = lib$(TARGET_BASENAME).so

TARGET_INCLUDE_FILES = \
	osgDB/DynamicLibrary\
	osgDB/Export\
	osgDB/FileNameUtils\
	osgDB/FileUtils\
	osgDB/ReaderWriter\
        osgDB/ReadFile\
	osgDB/Registry\
	osgDB/Version\
        osgDB/WriteFile\
	osgDB/Input\
	osgDB/Output\
	osgDB/Field\
	osgDB/FieldReader\
	osgDB/FieldReaderIterator\
	osgDB/DotOsgWrapper\


#LIBS    = -L../../lib -losg -lGLU -lGL -lm -ldl
LIBS    = -L../../lib -losg -lGLU -lGL -lm $(DYNAMICLIBRARYLIB)

LIB	= ../../lib/lib$(TARGET_BASENAME).so
#LIB	= ../../lib/lib$(TARGET_BASENAME).a

C++FLAGS += -I ../../include

include ../../Make/makerules

