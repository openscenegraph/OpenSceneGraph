#!smake
include $(OSGHOME)/Make/makedefs

C++FILES = \
	$(wildcard *.cpp)

TARGET = $(OSGHOME)/bin/wxsgv

TARGET_BIN_FILES = wxsgv

C++FLAGS += -I$(OSGHOME)/include `wx-config --cflags`

#note, use this library list when using the Performer osgPlugin.
#LIBS = ${PFLIBS} -losgWX -losgUtil -losgDB -losg \
#       `wx-config --libs` -lwx_gtk_gl \
#       -lGLU -lGL  -lm -lXmu  -lX11 -lXi

#note, standard library list. 
LIBS = -losgWX -losgUtil -losgDB -losg \
       `wx-config --libs` -lwx_gtk_gl \
	   $(GL_LIBS) $(X_LIBS)

C++FLAGS += -I$(OSGHOME)/include -I./icons
LDFLAGS += -L$(OSGHOME)/lib

include $(OSGHOME)/Make/makerules

