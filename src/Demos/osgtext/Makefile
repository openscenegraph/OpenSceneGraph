#!smake
include $(TOP)/Make/makedefs

C++FILES = \
	main.cpp

TARGET = $(TOP)/bin/osgtext

TARGET_BIN_FILES = osgtext

#note, use this library list when using the Performer osgPlugin.
#LIBS = ${PFLIBS} -losgGLUT -losgUtil -losgDB -losg $(GLUTLIB) -lGLU -lGL  -lm -lXmu  -lX11 -lXi

#note, standard library list. 
LIBS = -losgGLUT -losgText -losgUtil -losgDB -losg $(GLUTLIB) $(GL_LIBS) $(X_LIBS)

C++FLAGS += -I$(TOP)/include -g
LDFLAGS += -L$(TOP)/lib -L/usr/freeware/lib32

include $(TOP)/Make/makerules

