#!smake
include ../../Make/makedefs

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


LIBS    = -L../../lib -lfreetype -losgDB -losg -lGLU -lGL -lm

LIB	= ../../lib/lib$(TARGET_BASENAME).so
#LIB	= ../../lib/lib$(TARGET_BASENAME).a

TARGET_LIB_FILES = lib$(TARGET_BASENAME).so
TARGET_INCLUDE_FILES = \
	osgText/Export\
	osgText/Font\
	osgText/Paragraph\
	osgText/Text\
	osgText/Version

C++FLAGS += -I../../include -I/usr/include/freetype2 -I/usr/local/include -I/usr/local/include/freetype2 
LDFLAGS += -L/usr/local/lib

include ../../Make/makerules

