TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
        ReaderWriterDW.cpp\

LIBS  += $(OSG_LIBS) $(GL_LIBS) $(OTHER_LIBS)

TARGET_BASENAME = dw
include $(TOPDIR)/Make/cygwin_plugin_def
PLUGIN = $(PLUGIN_PREFIX)$(TARGET_BASENAME).$(PLUGIN_EXT)

include $(TOPDIR)/Make/makerules 
