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

#under Darwin we have to use the framework stuff to get GLUT OpenGL etc.
MACOSXLIBS = -losgGLUT -losgUtil -losgDB -losg -lm -ldl -lstdc++ -lobjc

C++FLAGS += -I$(OSGHOME)/include
LDFLAGS += -L$(OSGHOME)/lib

include $(OSGHOME)/Make/makerules

test :
	@echo no current test for osgconv:
