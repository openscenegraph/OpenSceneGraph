#!smake
include ../../../Make/makedefs

C++FILES = \
	sgv.cpp

TARGET = ../../../bin/sgv

TARGET_BIN_FILES = sgv

# LIBS = -losg -lglut -lGLU -lGL  -lm -lXmu  -lX11 -lXi
# C++FLAGS += -I../../include
# LDFLAGS += -L../../lib -L/usr/X11R6/lib

LIBS = -losgGLUT -losgUtil -losg -lglut -lGLU -lGL  -lm -lXmu  -lX11 -lXi
C++FLAGS += -I../../../include
LDFLAGS += -L../../../lib -L/usr/X11R6/lib

include ../../../Make/makerules
