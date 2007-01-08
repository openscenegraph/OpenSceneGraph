TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
        TXFFont.cpp\
        ReaderWriterTXF.cpp\

LIBS   += -losgText -losg -losgDB $(OTHER_LIBS) 


TARGET_BASENAME = txf
include $(TOPDIR)/Make/cygwin_plugin_def
PLUGIN = $(PLUGIN_PREFIX)$(TARGET_BASENAME).$(PLUGIN_EXT)

include $(TOPDIR)/Make/makerules 
