TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	ReaderWriterNET.cpp\
	sockinet.cpp\
	sockstream.cpp\

LIBS  += $(OSG_LIBS) $(OTHER_LIBS) $(SOCKET_LIBS)

TARGET_BASENAME = net
include $(TOPDIR)/Make/cygwin_plugin_def
PLUGIN = $(PLUGIN_PREFIX)$(TARGET_BASENAME).$(PLUGIN_EXT)

include $(TOPDIR)/Make/makerules 
