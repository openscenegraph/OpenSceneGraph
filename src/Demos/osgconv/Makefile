#!smake
include $(OSGHOME)/Make/makedefs

C++FILES = \
	osgconv.cpp\
	OrientationConverter.cpp


TARGET = $(OSGHOME)/bin/osgconv

TARGET_BIN_FILES = osgconv

#note, use this library list when using the Performer osgPlugin.
#LIBS = ${PFLIBS} -losgDB -losg -lGLU -lGL  -lm -lXmu  -lX11 -lXi

#note, standard library list. 
LIBS = -losgDB -losg $(GL_LIBS) $(X_LIBS)

C++FLAGS += -I$(OSGHOME)/include
LDFLAGS += -L$(OSGHOME)/lib

include $(OSGHOME)/Make/makerules
