TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	Attributes.cpp\
	Comparator.cpp\
	ConstructorInfo.cpp\
	Converter.cpp\
	ConverterProxy.cpp\
	CustomAttribute.cpp\
	CustomAttributeProvider.cpp\
	Exceptions.cpp\
	MethodInfo.cpp\
	ParameterInfo.cpp\
	PropertyInfo.cpp\
	ReaderWriter.cpp\
	Reflection.cpp\
	Reflector.cpp\
	Type.cpp\
	Value.cpp\

LIBS   += -losgIntrospection -losg -losgIntrospection  $(GL_LIBS) $(OTHER_LIBS)

OPTF = -O

TARGET_BASENAME = osgIntrospection
include $(TOPDIR)/Make/cygwin_wrapper_def
WRAPPER = $(WRAPPER_PREFIX)$(TARGET_BASENAME).$(PLUGIN_EXT)

include $(TOPDIR)/Make/makerules 
