#!smake
include ../../../Make/makedefs

C++FILES = \
        ReaderWriterPNG.cpp\

LIB = ../../../lib/osgPlugins/osgdb_png.so

TARGET_LOADER_FILES = osgPlugins/osgdb_png.so

LIBS = -lpng
C++FLAGS += -I../../../include -I/usr/local/include -I/usr/freeware/include
LDFLAGS += -L../../../lib -L/usr/local/lib -L/usr/freeware/lib32

include ../../../Make/makerules



