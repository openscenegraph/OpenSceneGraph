#!smake
include ../../Make/makedefs

C++FILES = \
        ConvertFromPerformer.cpp\
        pf2osg.cpp

C++FLAGS += -I../../include -O


LIBS = -losg 

TARGET = ../../bin/pf2osg

LDFLAGS += -L../../lib -L/usr/X11R6/lib

LIBS = -losg -lGLU -lGL  -lm ${PFLIBS} 

include ../../Make/makerules

