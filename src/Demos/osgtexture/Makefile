#!smake
include ../../../Make/makedefs

C++FILES = \
	osgtexture.cpp

TARGET = ../../../bin/osgtexture

TARGET_BIN_FILES = osgtexture

#note, use this library list when using the Performer osgPlugin.
#LIBS = ${PFLIBS} -losgGLUT -losgUtil -losgDB -losg $(GLUTLIB) -lGLU -lGL  -lm -lXmu  -lX11 -lXi

#note, standard library list. 
LIBS = -losgGLUT -losgUtil -losgDB -losg $(GLUTLIB) -lGLU -lGL  -lm -lXmu  -lX11 -lXi

C++FLAGS += -I../../../include
LDFLAGS += -L../../../lib

include ../../../Make/makerules
