#!gmake
include $(OSGHOME)/Make/makedefs

C++FILES = \
	main.cpp

TARGET = $(OSGHOME)/bin/osgtext

TARGET_BIN_FILES = osgtext

#note, use this library list when using the Performer osgPlugin.
#LIBS = ${PFLIBS} -losgGLUT -losgUtil -losgDB -losg $(GLUTLIB) -lGLU -lGL  -lm -lXmu  -lX11 -lXi

#note, standard library list. 
LIBS = -losgGLUT -losgText -losgUtil -losgDB -losg $(GLUTLIB) $(GL_LIBS) $(X_LIBS)
MACOSXLIBS = -losgGLUT -losgText -losgUtil -losgDB -losg -lfreetype -ldl -lobjc -lstdc++

C++FLAGS += -I$(OSGHOME)/include -I. 
LDFLAGS += -L$(OSGHOME)/lib -L/usr/freeware/lib32


include $(OSGHOME)/Make/makerules

test :
	osgtext 
