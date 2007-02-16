TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	GraphicsContextImplementation.cpp\
	KeyboardMouseCallback.cpp\
	OsgCameraGroup.cpp\
	OsgSceneHandler.cpp\
	ViewerEventHandler.cpp\

LIBS   += -losgProducer -losg -losgIntrospection -losgUtil -losgDB -lProducer -losgGA -losgText  $(GL_LIBS) $(OTHER_LIBS)

OPTF = -O

TARGET_BASENAME = osgProducer
include $(TOPDIR)/Make/cygwin_wrapper_def
WRAPPER = $(WRAPPER_PREFIX)$(TARGET_BASENAME).$(PLUGIN_EXT)

include $(TOPDIR)/Make/makerules 
