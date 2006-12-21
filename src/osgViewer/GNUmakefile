TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 


CXXFILES = \
	SimpleViewer.cpp\
	View.cpp\
	Scene.cpp\
	Viewer.cpp\
	CompositeViewer.cpp\
	Version.cpp\


CXXFILES += GraphicsWindowX11.cpp\


DEF     += -DOSGVIEWER_LIBRARY

LIBS  += -losgGA -losgDB -losgUtil -losg $(GL_LIBS) $(OTHER_LIBS)

TARGET_BASENAME = osgViewer
LIB = $(LIB_PREFIX)$(TARGET_BASENAME).$(LIB_EXT)

include $(TOPDIR)/Make/makerules 
