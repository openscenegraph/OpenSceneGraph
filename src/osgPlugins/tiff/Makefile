#!smake
include ../../../Make/makedefs

C++FILES = \
        ReaderWriterTIFF.cpp\

LIB = ../../../lib/osgPlugins/osgdb_tiff.so

TARGET_LOADER_FILES = osgPlugins/osgdb_tiff.so

LIBS = -ltiff
C++FLAGS += -I. -I../../../include
LDFLAGS += -L../../../lib $(FREEBSD_LOCALLIBS)

include ../../../Make/makerules



