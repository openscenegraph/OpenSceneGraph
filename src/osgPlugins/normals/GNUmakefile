TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	ReaderWriterNormals.cpp\
    Normals.cpp\

LIBS  += $(OSG_LIBS) $(OTHER_LIBS) $(SOCKET_LIBS)

TARGET_BASENAME = normals
include $(TOPDIR)/Make/cygwin_plugin_def
PLUGIN = $(PLUGIN_PREFIX)$(TARGET_BASENAME).$(PLUGIN_EXT)

include $(TOPDIR)/Make/makerules 
