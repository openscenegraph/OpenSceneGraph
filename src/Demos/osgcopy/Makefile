#!gmake
include $(OSGHOME)/Make/makedefs

C++FILES = \
	osgcopy.cpp

TARGET = $(OSGHOME)/bin/osgcopy

TARGET_BIN_FILES = osgcopy

#note, standard library list. 
LIBS = -losgGLUT -losgUtil -losgDB -losg $(GLUTLIB) $(GL_LIBS) $(X_LIBS)

#under Darwin we have to use the framework stuff to get GLUT OpenGL etc.
MACOSXLIBS = -losgGLUT -losgUtil -losgDB -losg -lm -ldl -lstdc++ -lobjc


C++FLAGS += -I$(OSGHOME)/include
LDFLAGS += -L$(OSGHOME)/lib

include $(OSGHOME)/Make/makerules

test :
	@echo no current test for osgconv:
