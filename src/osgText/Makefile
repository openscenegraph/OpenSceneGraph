#!gmake
include $(OSGHOME)/Make/makedefs

C++FILES = \
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


TARGET_BASENAME = osgText


LIBS    = -L$(OSGHOME)/lib -lfreetype -losgDB -losg $(GL_LIBS)
MACOSXLIBS    = -L$(OSGHOME)/lib -lfreetype -losgDB -losg -lstdc++

LIB	= $(OSGHOME)/lib/lib$(TARGET_BASENAME).$(SO_EXT)
#LIB	= $(OSGHOME)/lib/lib$(TARGET_BASENAME).a

TARGET_LIB_FILES = lib$(TARGET_BASENAME).$(SO_EXT)
TARGET_INCLUDE_FILES = \
	osgText/Export\
	osgText/Font\
	osgText/Paragraph\
	osgText/Text\
	osgText/Version

C++FLAGS += -I$(OSGHOME)/include \
		-I/usr/include/freetype2 \
		-I/usr/local/include \
		-I/usr/local/include/freetype2  \
		-I/usr/freeware/include \
		-I/usr/freeware/include/freetype2 \
		-DOSGTEXT_LIBRARY

LDFLAGS += -L/usr/local/lib -L/usr/freeware/lib32

include $(OSGHOME)/Make/makerules

