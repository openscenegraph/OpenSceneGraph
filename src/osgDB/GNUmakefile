TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
        DatabasePager.cpp\
	DynamicLibrary.cpp\
	FileNameUtils.cpp\
	FileUtils.cpp\
	ReadFile.cpp\
	Registry.cpp\
	Version.cpp\
	WriteFile.cpp\
	ImageOptions.cpp\
	Input.cpp\
	Output.cpp\
	Field.cpp\
	FieldReader.cpp\
	FieldReaderIterator.cpp\
	DotOsgWrapper.cpp\


LIBS += -losg $(OTHER_LIBS) $(DYNAMICLIBRARYLIB) 
DEF  += -DOSGDB_LIBRARY

TARGET_BASENAME = osgDB
LIB = $(LIB_PREFIX)$(TARGET_BASENAME).$(LIB_EXT)

include $(TOPDIR)/Make/makerules 
