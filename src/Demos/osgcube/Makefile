#!smake
include $(TOP)/Make/makedefs

C++FILES = \
	osgcube.cpp

TARGET = $(TOP)/bin/osgcube

TARGET_BIN_FILES = osgcube

LIBS = -losgGLUT -losgDB -losgUtil -losg $(GLUTLIB) $(GL_LIBS) $(X_LIBS)
C++FLAGS += -I$(TOP)/include
LDFLAGS += -L$(TOP)/lib

include $(TOP)/Make/makerules
