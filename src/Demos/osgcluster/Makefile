#!gmake
include $(OSGHOME)/Make/makedefs

C++FILES = \
        broadcaster.cpp\
	receiver.cpp\
	osgcluster.cpp

C++FLAGS += -g 

TARGET = $(OSGHOME)/bin/osgcluster

TARGET_BIN_FILES = osgcluster

#note, use this library list when using the Performer osgPlugin.
#LIBS = ${PFLIBS} -losgGLUT -losgUtil -losgDB -losg $(GLUTLIB) -lGLU -lGL  -lm -lXmu  -lX11 -lXi

#note, standard library list. 
LIBS = -losgGLUT -losgUtil -losgDB -losg $(GLUTLIB) $(GL_LIBS) $(X_LIBS)

C++FLAGS += -I$(OSGHOME)/include
LDFLAGS += -L$(OSGHOME)/lib

include $(OSGHOME)/Make/makerules

test :
	@echo no current test for osgconv:
