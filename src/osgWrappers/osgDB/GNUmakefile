TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	Archive.cpp\
	DatabasePager.cpp\
	DotOsgWrapper.cpp\
	DynamicLibrary.cpp\
	Field.cpp\
	FieldReader.cpp\
	FieldReaderIterator.cpp\
	FileUtils.cpp\
	ImageOptions.cpp\
	Input.cpp\
	Output.cpp\
	ParameterOutput.cpp\
	ReaderWriter.cpp\
	ReentrantMutex.cpp\
	Registry.cpp\
	SharedStateManager.cpp\

LIBS   += -losgDB -losg -losgIntrospection  $(GL_LIBS) $(OTHER_LIBS)

OPTF = -O

TARGET_BASENAME = osgDB
include $(TOPDIR)/Make/cygwin_wrapper_def
WRAPPER = $(WRAPPER_PREFIX)$(TARGET_BASENAME).$(PLUGIN_EXT)

include $(TOPDIR)/Make/makerules 
