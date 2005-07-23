TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	EventAdapter.cpp\
	GraphicsContextImplementation.cpp\
	KeyboardMouseCallback.cpp\
	OsgCameraGroup.cpp\
	OsgSceneHandler.cpp\
	Viewer.cpp\
	ViewerEventHandler.cpp\

LIBS   += -losgProducer -losg -losgIntrospection  $(GL_LIBS) $(OTHER_LIBS)

OPTF = -O

TARGET_BASENAME = osgProducer
include $(TOPDIR)/Make/cygwin_wrapper_def
WRAPPER = $(WRAPPER_PREFIX)$(TARGET_BASENAME).$(PLUGIN_EXT)

include $(TOPDIR)/Make/makerules 
