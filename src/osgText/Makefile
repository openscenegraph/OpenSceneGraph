#!smake
include ../../Make/makedefs

C++FILES = \
	FTBitmapGlyph.cpp \
        FTGLOutlineFont.cpp \
        FTGlyphContainer.cpp \
        FTSize.cpp \
	FTCharmap.cpp \
        FTGLPixmapFont.cpp \
        FTLibrary.cpp \
        FTTextureGlyph.cpp \
	FTFace.cpp \
        FTGLPolygonFont.cpp \
        FTOutlineGlyph.cpp \
        FTVectoriser.cpp \
	FTFont.cpp \
        FTGLTextureFont.cpp \
        FTPixmapGlyph.cpp \
	FTGLBitmapFont.cpp \
        FTGlyph.cpp \
        FTPolyGlyph.cpp \
        FTVectorGlyph.cpp \
        Text.cpp \
        Version.cpp


TARGET_BASENAME = osgText


LIBS    = -L../../lib -lfreetype -losgDB -losg -lGLU -lGL -lm

LIB	= ../../lib/lib$(TARGET_BASENAME).so
#LIB	= ../../lib/lib$(TARGET_BASENAME).a

TARGET_LIB_FILES = lib$(TARGET_BASENAME).so
TARGET_INCLUDE_FILES = \
	osgText/Export\
	osgText/Version\
	osgText/Text

C++FLAGS += -I ../../include -I /usr/include/freetype2  -I /usr/local/include/freetype2 

include ../../Make/makerules

