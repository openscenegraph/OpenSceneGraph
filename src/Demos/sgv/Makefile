#!smake
include $(OSGHOME)/Make/makedefs

C++FILES = \
	sgv.cpp

TARGET = $(OSGHOME)/bin/sgv

TARGET_BIN_FILES = sgv

#note, standard library list. 
LIBS = -losgGLUT -losgUtil -losgDB -losg $(GLUTLIB) $(GL_LIBS) $(X_LIBS)

#under Darwin we have to use the framework stuff to get GLUT OpenGL etc.
MACOSXLIBS = -losgGLUT -losgUtil -losgDB -losg -lm -ldl -lstdc++ -lobjc


C++FLAGS += -I$(OSGHOME)/include
LDFLAGS += -L$(OSGHOME)/lib

include $(OSGHOME)/Make/makerules

