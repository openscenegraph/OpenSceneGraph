#!gmake
include $(OSGHOME)/Make/makedefs

C++FILES = \
	osgcube.cpp

TARGET = $(OSGHOME)/bin/osgcube

TARGET_BIN_FILES = osgcube

LIBS = -losgGLUT -losgDB -losgUtil -losg $(GLUTLIB) $(GL_LIBS) $(X_LIBS)

#under Darwin we have to use the framework stuff to get GLUT OpenGL etc.
MACOSXLIBS = -losgGLUT -losgUtil -losgDB -losg -lm -ldl -lstdc++ -lobjc

C++FLAGS += -I$(OSGHOME)/include
LDFLAGS += -L$(OSGHOME)/lib

include $(OSGHOME)/Make/makerules

test :
	osgcube
