TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	DynamicLibrary.cpp\
	FileNameUtils.cpp\
	FileUtils.cpp\
	ReadFile.cpp\
	Registry.cpp\
	Version.cpp\
	WriteFile.cpp\
	Input.cpp\
	Output.cpp\
	ParameterOutput.cpp\
	Field.cpp\
	FieldReader.cpp\
	FieldReaderIterator.cpp\
	DotOsgWrapper.cpp\


LIBS += -losg $(OTHER_LIBS) $(DYNAMICLIBRARYLIB) 
DEF  += -DOSGDB_LIBRARY

TARGET_BASENAME = osgDB
LIB = $(LIB_PREFIX)$(TARGET_BASENAME).$(LIB_EXT)

include $(TOPDIR)/Make/makerules 
