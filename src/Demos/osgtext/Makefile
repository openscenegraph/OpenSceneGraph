#!smake
include ../../../Make/makedefs

C++FILES = \
	main.cpp

TARGET = ../../../bin/osgtext

TARGET_BIN_FILES = osgtext

#note, use this library list when using the Performer osgPlugin.
#LIBS = ${PFLIBS} -losgGLUT -losgUtil -losgDB -losg $(GLUTLIB) -lGLU -lGL  -lm -lXmu  -lX11 -lXi

#note, standard library list. 
LIBS = -losgGLUT -losgText -losgUtil -losgDB -losg $(GLUTLIB) -lGLU -lGL  -lm -lXmu  -lX11 -lXi

C++FLAGS += -I../../../include -g
LDFLAGS += -L../../../lib

include ../../../Make/makerules

