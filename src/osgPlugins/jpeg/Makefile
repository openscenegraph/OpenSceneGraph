#!smake
include ../../../Make/makedefs

C++FILES = \
        ReaderWriterJPEG.cpp\

LIB = ../../../lib/osgPlugins/osgdb_jpeg.so

TARGET_LOADER_FILES = osgPlugins/osgdb_jpeg.so

LIBS = -ljpeg 
C++FLAGS += -I. -I../../../include
LDFLAGS += -L../../../lib $(FREEBSD_LOCALLIBS)

include ../../../Make/makerules



