#!smake
include $(OSGHOME)/Make/makedefs

C++FILES = \
	osgviews.cpp

TARGET = $(OSGHOME)/bin/osgviews

TARGET_BIN_FILES = osgviews

#note, use this library list when using the Performer osgPlugin.
#LIBS = ${PFLIBS} -losgGLUT -losgUtil -losgDB -losg $(GLUTLIB) -lGLU -lGL  -lm -lXmu  -lX11 -lXi

#note, standard library list. 
LIBS = -losgGLUT -losgUtil -losgDB -losg $(GLUTLIB) $(GL_LIBS) $(X_LIBS)

C++FLAGS += -I$(OSGHOME)/include
LDFLAGS += -L$(OSGHOME)/lib

include $(OSGHOME)/Make/makerules

