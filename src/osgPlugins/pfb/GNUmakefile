TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
        ConvertFromPerformer.cpp\
        ReaderWriterPFB.cpp\

LIBS   += $(OSG_LIBS) $(PF_XTRA_LIBS) $(OTHER_LIBS)

# Performer libs must be compiled -O on Linux
DBGF   += -O

TARGET_BASENAME = pfb
include $(TOPDIR)/Make/cygwin_plugin_def
PLUGIN = $(PLUGIN_PREFIX)$(TARGET_BASENAME).$(PLUGIN_EXT)

include $(TOPDIR)/Make/makerules 
