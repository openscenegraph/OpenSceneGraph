#!smake
include ../../../Make/makedefs

C++FILES = \
	osgviews.cpp

TARGET = ../../../bin/osgviews

TARGET_BIN_FILES = osgviews

#note, use this library list when using the Performer osgPlugin.
#LIBS = ${PFLIBS} -losgGLUT -losgUtil -losgDB -losg $(GLUTLIB) -lGLU -lGL  -lm -lXmu  -lX11 -lXi

#note, standard library list. 
LIBS = -losgGLUT -losgUtil -losgDB -losg $(GLUTLIB) -lGLU -lGL  -lm -lXmu  -lX11 -lXi

C++FLAGS += -I../../../include
LDFLAGS += -L../../../lib

include ../../../Make/makerules

