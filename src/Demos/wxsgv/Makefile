#!smake
include $(TOP)/Make/makedefs

C++FILES = \
	$(wildcard *.cpp)

TARGET = $(TOP)/bin/wxsgv

TARGET_BIN_FILES = wxsgv

C++FLAGS += -I$(TOP)/include `wx-config --cflags`

#note, use this library list when using the Performer osgPlugin.
#LIBS = ${PFLIBS} -losgWX -losgUtil -losgDB -losg \
#       `wx-config --libs` -lwx_gtk_gl \
#       -lGLU -lGL  -lm -lXmu  -lX11 -lXi

#note, standard library list. 
LIBS = -losgWX -losgUtil -losgDB -losg \
       `wx-config --libs` -lwx_gtk_gl \
	   $(GL_LIBS) $(X_LIBS)

C++FLAGS += -I$(TOP)/include -I./icons
LDFLAGS += -L$(TOP)/lib

include $(TOP)/Make/makerules

