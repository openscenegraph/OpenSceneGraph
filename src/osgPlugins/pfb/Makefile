#!smake
include ../../../Make/makedefs

CFILES = \

C++FILES = \
        ConvertFromPerformer.cpp\
        ConvertToPerformer.cpp\
        ReaderWriterPFB.cpp\

LIB = ../../../lib/osgPlugins/osgdb_pfb.so

LIBS = -losg -losgDB $(PFLIBS)

TARGET_LOADER_FILES = osgPlugins/osgdb_pfb.so

C++FLAGS += -g -I../../../include 

LDFLAGS += -g -L../../../lib  -L/usr/X11R6/lib

PREREQUISITES = preq

include ../../../Make/makerules


preq :
	[ -f /usr/lib/blah ] || exit 1



