#!smake
include $(OSGHOME)/Make/makedefs

C++FILES = \
	osgimpostor.cpp

TARGET = $(OSGHOME)/bin/osgimpostor

TARGET_BIN_FILES = osgimpostor

#note, use this library list when using the Performer osgPlugin.
#LIBS = ${PFLIBS} -losgGLUT -losgUtil -losgDB -losg $(GLUTLIB) -lGLU -lGL  -lm -lXmu  -lX11 -lXi

#note, standard library list. 
LIBS = -losgGLUT -losgUtil -losgDB -losg $(GLUTLIB) $(GL_LIBS) $(X_LIBS)

#under Darwin we have to use the framework stuff to get GLUT OpenGL etc.
MACOSXLIBS = -losgGLUT -losgUtil -losgDB -losg -lm -ldl -lstdc++ -lobjc

C++FLAGS += -I$(OSGHOME)/include
LDFLAGS += -L$(OSGHOME)/lib

include $(OSGHOME)/Make/makerules

test :
	osgimpostor Town.osg
