#!smake
include $(TOP)/Make/makedefs

C++FILES = \
	sgv.cpp

TARGET = $(TOP)/bin/sgv

TARGET_BIN_FILES = sgv

#note, standard library list. 
LIBS = -losgGLUT -losgUtil -losgDB -losg $(GLUTLIB) $(GL_LIBS) $(X_LIBS)

C++FLAGS += -I$(TOP)/include
LDFLAGS += -L$(TOP)/lib

include $(TOP)/Make/makerules

