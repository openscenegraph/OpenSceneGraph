#!smake
include $(OSGHOME)/Make/makedefs

C++FILES = \
	osgcube.cpp

TARGET = $(OSGHOME)/bin/osgcube

TARGET_BIN_FILES = osgcube

LIBS = -losgGLUT -losgDB -losgUtil -losg $(GLUTLIB) $(GL_LIBS) $(X_LIBS)
C++FLAGS += -I$(OSGHOME)/include
LDFLAGS += -L$(OSGHOME)/lib

include $(OSGHOME)/Make/makerules
