#!smake
include ../../../Make/makedefs

C++FILES = \
	cube.cpp

TARGET = ../../../bin/cube

TARGET_BIN_FILES = cube

LIBS = -losgGLUT -losgUtil -losg -lglut -lGLU -lGL  -lm -lXmu  -lX11 -lXi
C++FLAGS += -I../../../include
LDFLAGS += -L../../../lib -L/usr/X11R6/lib

include ../../../Make/makerules
