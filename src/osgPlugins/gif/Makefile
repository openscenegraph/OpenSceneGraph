#!smake
include ../../../Make/makedefs

C++FILES = \
        ReaderWriterGIF.cpp\

LIB = ../../../lib/osgPlugins/osgdb_gif.$(SO_EXT)

TARGET_LOADER_FILES = osgPlugins/osgdb_gif.$(SO_EXT)

LIBS = -lungif

C++FLAGS += -I. -I../../../include  -I/usr/freeware/include
LDFLAGS += -L../../../lib $(FREEBSD_LOCALLIBS) -L/usr/freeware/lib32

include ../../../Make/makerules



