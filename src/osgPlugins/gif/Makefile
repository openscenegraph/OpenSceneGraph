#!smake
include ../../../Make/makedefs

C++FILES = \
        ReaderWriterGIF.cpp\

LIB = ../../../lib/osgPlugins/osgdb_gif.so

TARGET_LOADER_FILES = osgPlugins/osgdb_gif.so

LIBS = -lungif

C++FLAGS += -I. -I../../../include 
LDFLAGS += -L../../../lib $(FREEBSD_LOCALLIBS)

include ../../../Make/makerules



