#!smake
include ../../../Make/makedefs

C++FILES = \
	sgv.cpp

TARGET = ../../../bin/sgv

TARGET_BIN_FILES = sgv

#note, standard library list. 
LIBS = -losgGLUT -losgUtil -losgDB -losg $(GLUTLIB) -lGLU -lGL  -lm -lXmu  -lX11 -lXi

C++FLAGS += -I../../../include -g
LDFLAGS += -L../../../lib

include ../../../Make/makerules

