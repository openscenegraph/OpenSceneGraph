TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	BlendFunc.cpp\
	Drawable.cpp\
	Geode.cpp\
	Geometry.cpp\
	Group.cpp\
	Material.cpp\
	Node.cpp\
	Object.cpp\
	StateAttribute.cpp\
	StateSet.cpp\
	Vec2.cpp\


LIBS   += -losg -losgIntrospection $(OTHER_LIBS)

TARGET_BASENAME = osg
include $(TOPDIR)/Make/cygwin_wrapper_def
WRAPPER = $(WRAPPER_PREFIX)$(TARGET_BASENAME).$(PLUGIN_EXT)

include $(TOPDIR)/Make/makerules 
