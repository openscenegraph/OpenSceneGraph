TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	FTBitmapGlyph.cpp \
	FTCharmap.cpp \
	FTFace.cpp \
	FTFont.cpp \
	FTGLBitmapFont.cpp \
	FTGLOutlineFont.cpp \
	FTGLPixmapFont.cpp \
	FTGLPolygonFont.cpp \
	FTGLTextureFont.cpp \
	FTGlyphContainer.cpp \
	FTGlyph.cpp \
	FTLibrary.cpp \
	FTOutlineGlyph.cpp \
	FTPixmapGlyph.cpp \
	FTPolyGlyph.cpp \
	FTSize.cpp \
	FTTextureGlyph.cpp \
	FTVectoriser.cpp \
	Font.cpp \
	Paragraph.cpp \
	Text.cpp \
	Version.cpp

LIBS  += $(OSG_LIBS) $(FREETYPE_LIB) $(OTHER_LIBS)

INC += -I$(OSGHOME)/include \
	-I/usr/include/freetype2 \
	-I/usr/local/include \
	-I/usr/local/include/freetype2  \
	-I/usr/freeware/include \
	-I/usr/freeware/include/freetype2 \

DEF += -DOSGTEXT_LIBRARY
LINKARGS += -L/usr/local/lib\
	    -L/usr/freeware/lib$(ARCH)

LIB = libosgText.$(SO_EXT)

include $(TOPDIR)/Make/makerules 
