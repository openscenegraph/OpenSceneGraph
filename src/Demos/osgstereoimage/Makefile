#!gmake
include $(OSGHOME)/Make/makedefs

C++FILES = \
	osgstereoimage.cpp

TARGET = $(OSGHOME)/bin/osgstereoimage

TARGET_BIN_FILES = osgstereoimage

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
	osgstereoimage Images/dog_left_eye.jpg Images/dog_right_eye.jpg
