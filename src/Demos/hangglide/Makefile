#!gmake
include $(OSGHOME)/Make/makedefs

C++FILES = \
	hangglide.cpp\
	ReaderWriterFLY.cpp\
	GliderManipulator.cpp\
	hat.cpp\
	terrain.cpp\
	tank.cpp\
	sky.cpp\
	base.cpp\
	trees.cpp\

TARGET = $(OSGHOME)/bin/hangglide

TARGET_BIN_FILES = hangglide

#note, use this library list when using the Performer osgPlugin.
#LIBS = ${PFLIBS} -losgGLUT -losgUtil -losgDB -losg $(GLUTLIB) -lGLU -lGL  -lm -lXmu  -lX11 -lXi

#note, standard library list. 
LIBS = -losgGLUT -losgUtil -losgDB -losg $(GLUTLIB) $(GL_LIBS) $(X_LIBS)

#under Darwin we have to use the framework stuff to get GLUT OpenGL etc.
MACOSXLIBS = -losgGLUT -losgUtil -losgDB -losg -lm -ldl -lstdc++ -lobjc


# $(OSGDB_LIBS)
C++FLAGS += -I. -I$(OSGHOME)/include
LDFLAGS += -L$(OSGHOME)/lib

include $(OSGHOME)/Make/makerules

test :
	hangglide
