#!smake
include $(TOP)/Make/makedefs

C++FILES = \
	osgviews.cpp

TARGET = $(TOP)/bin/osgviews

TARGET_BIN_FILES = osgviews

#note, use this library list when using the Performer osgPlugin.
#LIBS = ${PFLIBS} -losgGLUT -losgUtil -losgDB -losg $(GLUTLIB) -lGLU -lGL  -lm -lXmu  -lX11 -lXi

#note, standard library list. 
LIBS = -losgGLUT -losgUtil -losgDB -losg $(GLUTLIB) $(GL_LIBS) $(X_LIBS)

C++FLAGS += -I$(TOP)/include
LDFLAGS += -L$(TOP)/lib

include $(TOP)/Make/makerules

