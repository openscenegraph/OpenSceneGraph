#!smake
include ../../../Make/makedefs

C++FILES = \
	hangglide.cpp\
	ReaderWriterFLY.cpp\
        GliderManipulator.cpp\
        hat.cpp\
	terrain.cpp\
	tank.cpp\
	sky.cpp\
	base.cpp\
	trees.cpp\

TARGET = ../../../bin/hangglide

TARGET_BIN_FILES = hangglide

#note, use this library list when using the Performer osgPlugin.
#LIBS = ${PFLIBS} -losgGLUT -losgUtil -losgDB -losg $(GLUTLIB) -lGLU -lGL  -lm -lXmu  -lX11 -lXi

#note, standard library list. 
LIBS = -losgGLUT -losgUtil -losgDB -losg $(GLUTLIB) -lGLU -lGL  -lm -lXmu  -lX11 -lXi

C++FLAGS += -I. -I../../../include
LDFLAGS += -L../../../lib

include ../../../Make/makerules
