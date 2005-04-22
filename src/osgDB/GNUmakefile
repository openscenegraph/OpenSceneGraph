TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
        Archive.cpp\
        DatabasePager.cpp\
	DynamicLibrary.cpp\
	FileNameUtils.cpp\
	FileUtils.cpp\
	ReadFile.cpp\
	ReaderWriter.cpp\
	ReentrantMutex.cpp\
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
	SharedStateManager.cpp\


LIBS += -losg $(GL_LIBS) $(OTHER_LIBS) $(DYNAMICLIBRARYLIB) 

ifeq ($(OS),Darwin)
LIBS += ${CARBON_LIB}
endif

ifeq ($(DARWIN_QUICKTIME),yes)
  DEF+= -DDARWIN_QUICKTIME
endif

ifeq ($(XINE_INSTALLED),yes)
  DEF+= -DUSE_XINE
endif


DEF  += -DOSGDB_LIBRARY

TARGET_BASENAME = osgDB
LIB = $(LIB_PREFIX)$(TARGET_BASENAME).$(LIB_EXT)

include $(TOPDIR)/Make/makerules 
