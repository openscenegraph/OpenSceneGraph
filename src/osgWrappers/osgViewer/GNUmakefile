TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	CompositeViewer.cpp\
	GraphicsWindow.cpp\
	HelpHandler.cpp\
	Scene.cpp\
	View.cpp\
	Viewer.cpp\

LIBS   += -losgViewer -losg -losgIntrospection -losgUtil -losgDB -losgGA -losgText  $(GL_LIBS) $(OTHER_LIBS)

OPTF = -O

TARGET_BASENAME = osgViewer
include $(TOPDIR)/Make/cygwin_wrapper_def
WRAPPER = $(WRAPPER_PREFIX)$(TARGET_BASENAME).$(PLUGIN_EXT)

include $(TOPDIR)/Make/makerules 
