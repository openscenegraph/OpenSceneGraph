#!smake
include ../../../Make/makedefs

C++FILES = \
	osgconv.cpp

TARGET = ../../../bin/osgconv

TARGET_BIN_FILES = osgconv

#note, use this library list when using the Performer osgPlugin.
#LIBS = ${PFLIBS} -losgDB -losg -lGLU -lGL  -lm -lXmu  -lX11 -lXi

#note, standard library list. 
LIBS = -losgDB -losg -lGLU -lGL  -lm -lXmu  -lX11 -lXi

C++FLAGS += -I../../../include
LDFLAGS += -L../../../lib

include ../../../Make/makerules
