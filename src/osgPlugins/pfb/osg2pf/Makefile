#!smake
include ../../Make/makedefs

C++FILES = \
        ConvertToPerformer.cpp\
        osg2pf.cpp

C++FLAGS += -I../../include -O


LIBS = -losg 

TARGET = ../../bin/osg2pf

LDFLAGS += -L../../lib -L/usr/X11R6/lib

LIBS = -losg -lGLU  -lm ${PFLIBS}

include ../../Make/makerules

