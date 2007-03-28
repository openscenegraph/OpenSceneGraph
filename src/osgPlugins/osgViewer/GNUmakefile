TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	View.cpp\
	ReaderWriterOsgViewer.cpp\

LIBS   += -losgViewer -losgUtil -losgDB -losgGA -losgText -losg $(OTHER_LIBS)

TARGET_BASENAME = osgViewer
include $(TOPDIR)/Make/cygwin_plugin_def
PLUGIN = $(PLUGIN_PREFIX)$(TARGET_BASENAME).$(PLUGIN_EXT)

include $(TOPDIR)/Make/makerules 
