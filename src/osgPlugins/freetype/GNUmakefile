TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
        FreeTypeLibrary.cpp\
        FreeTypeFont.cpp\
        ReaderWriterFreeType.cpp\

LIBS   += -losgText $(OSG_LIBS) $(FREETYPE_LIB) $(OTHER_LIBS)

ifneq ($(OS),HP-UX)
INC += -I$(OSGHOME)/include \
    -I/usr/include/freetype2 \
    -I/usr/local/include \
    -I/usr/local/include/freetype2  \
    -I/usr/freeware/include \
    -I/usr/freeware/include/freetype2
endif
INC += $(FREETYPE_INCLUDE)

TARGET_BASENAME = freetype
include $(TOPDIR)/Make/cygwin_plugin_def
PLUGIN = $(PLUGIN_PREFIX)$(TARGET_BASENAME).$(PLUGIN_EXT)

include $(TOPDIR)/Make/makerules 
