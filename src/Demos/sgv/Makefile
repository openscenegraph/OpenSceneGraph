#!smake
include $(OSGHOME)/Make/makedefs

C++FILES = \
	sgv.cpp

TARGET = $(OSGHOME)/bin/sgv

TARGET_BIN_FILES = sgv

#note, standard library list. 
LIBS = -losgGLUT -losgUtil -losgDB -losg $(GLUTLIB) $(GL_LIBS) $(X_LIBS)

C++FLAGS += -I$(OSGHOME)/include
LDFLAGS += -L$(OSGHOME)/lib

include $(OSGHOME)/Make/makerules

