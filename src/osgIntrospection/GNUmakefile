TOPDIR = ../..
include $(TOPDIR)/Make/makedefs

CXXFILES =\
	ConstructorInfo.cpp\
	CustomAttributeProvider.cpp\
	DefaultReflectors.cpp\
	MethodInfo.cpp\
	PropertyInfo.cpp\
	Reflection.cpp\
	Type.cpp\
	Utility.cpp\
	Value.cpp\


LIBS += $(OTHER_LIBS) $(DYNAMICLIBRARYLIB)
DEF  += -DOSGINTROSPECTION_LIBRARY

TARGET_BASENAME = osgIntrospection
LIB = $(LIB_PREFIX)$(TARGET_BASENAME).$(LIB_EXT)

include $(TOPDIR)/Make/makerules
