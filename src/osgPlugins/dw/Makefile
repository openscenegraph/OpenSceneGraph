#!smake
include ../../../Make/makedefs

C++FILES = ReaderWriterDW.cpp

LIB = ../../../lib/osgPlugins/osgdb_dw.so

TARGET_LOADER_FILES = osgPlugins/osgdb_dw.so

LIBS = -losg -losgDB

C++FLAGS += -I. -I../../../include -DGLU_VERSION_1_2
LDFLAGS += -L../../../lib

include ../../../Make/makerules
