#!smake
include ../../../Make/makedefs

C++FILES = ReaderWriterLWO.cpp lw.cpp

LIB = ../../../lib/osgPlugins/osgdb_lwo.so

TARGET_LOADER_FILES = osgPlugins/osgdb_lwo.so

LIBS = -losg -losgDB -losgUtil

C++FLAGS += -I. -I../../../include 
LDFLAGS += -L../../../lib

include ../../../Make/makerules
