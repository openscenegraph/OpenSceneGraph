TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
        FreeTypeLibrary.cpp\
        FreeTypeFont.cpp\
        ReaderWriterFreeType.cpp\

INC += $(FREETYPE_INCLUDE)

LIBS   += -losgText $(OSG_LIBS) $(FREETYPE_LIB) $(OTHER_LIBS) 


TARGET_BASENAME = freetype
include $(TOPDIR)/Make/cygwin_plugin_def
PLUGIN = $(PLUGIN_PREFIX)$(TARGET_BASENAME).$(PLUGIN_EXT)

include $(TOPDIR)/Make/makerules 
