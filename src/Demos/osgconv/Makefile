#!smake
include $(TOP)/Make/makedefs

C++FILES = \
	osgconv.cpp\
	OrientationConverter.cpp


TARGET = $(TOP)/bin/osgconv

TARGET_BIN_FILES = osgconv

#note, use this library list when using the Performer osgPlugin.
#LIBS = ${PFLIBS} -losgDB -losg -lGLU -lGL  -lm -lXmu  -lX11 -lXi

#note, standard library list. 
LIBS = -losgDB -losg $(GL_LIBS) $(X_LIBS)

C++FLAGS += -I$(TOP)/include
LDFLAGS += -L$(TOP)/lib

include $(TOP)/Make/makerules
