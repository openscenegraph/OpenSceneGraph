#!smake
include ../../../Make/makedefs

C++FILES = \
	osgcube.cpp

TARGET = ../../../bin/osgcube

TARGET_BIN_FILES = osgcube

LIBS = -losgGLUT -losgDB -losgUtil -losg $(GLUTLIB) -lGLU -lGL  -lm -lXmu  -lX11 -lXi
C++FLAGS += -I../../../include
LDFLAGS += -L../../../lib

include ../../../Make/makerules
