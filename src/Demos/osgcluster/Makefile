#!smake
include $(TOP)/Make/makedefs

C++FILES = \
        broadcaster.cpp\
	receiver.cpp\
	osgcluster.cpp

C++FLAGS += -g 

TARGET = $(TOP)/bin/osgcluster

TARGET_BIN_FILES = osgcluster

#note, use this library list when using the Performer osgPlugin.
#LIBS = ${PFLIBS} -losgGLUT -losgUtil -losgDB -losg $(GLUTLIB) -lGLU -lGL  -lm -lXmu  -lX11 -lXi

#note, standard library list. 
LIBS = -losgGLUT -losgUtil -losgDB -losg $(GLUTLIB) $(GL_LIBS) $(X_LIBS)

C++FLAGS += -I$(TOP)/include
LDFLAGS += -L$(TOP)/lib

include $(TOP)/Make/makerules

