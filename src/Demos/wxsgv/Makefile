#!smake
include ../../../Make/makedefs

C++FILES = \
	$(wildcard *.cpp)

TARGET = ../../../bin/wxsgv

TARGET_BIN_FILES = wxsgv

C++FLAGS += -I../../include `wx-config --cflags`

#note, use this library list when using the Performer osgPlugin.
#LIBS = ${PFLIBS} -losgWX -losgUtil -losgDB -losg \
#       `wx-config --libs` -lwx_gtk_gl \
#       -lGLU -lGL  -lm -lXmu  -lX11 -lXi

#note, standard library list. 
LIBS = -losgWX -losgUtil -losgDB -losg \
       `wx-config --libs` -lwx_gtk_gl \
       -lGLU -lGL  -lm -lXmu  -lX11 -lXi

C++FLAGS += -I../../../include -I./icons
LDFLAGS += -L../../../lib

include ../../../Make/makerules

