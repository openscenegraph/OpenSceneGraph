TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
        ReaderWriterPNG.cpp\

LIBS  += $(OSG_LIBS) $(PNG_LIBS) $(OTHER_LIBS)
ifneq ($(OS),HP-UX)
INC  += -I/usr/local/include\
	-I/usr/freeware/include\

LINKARGS += -L/usr/freeware/lib$(ARCH)
else
INC += $(PNG_INCLUDE)
endif

TARGET_BASENAME = png
include $(TOPDIR)/Make/cygwin_plugin_def
PLUGIN = $(PLUGIN_PREFIX)$(TARGET_BASENAME).$(PLUGIN_EXT)

include $(TOPDIR)/Make/makerules 
