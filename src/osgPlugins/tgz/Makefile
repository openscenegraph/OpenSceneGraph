#!gmake
include $(OSGHOME)/Make/makedefs

C++FILES = \
	ReaderWriterTGZ.cpp\

TARGET_BASENAME = osgdb_tgz

LIB =
LOADABLE = $(OSGHOME)/lib/osgPlugins/$(TARGET_BASENAME).$(DL_EXT)

TARGET_LOADER_FILES = osgPlugins/$(TARGET_BASENAME).$(DL_EXT)

LIBS    =  $(GL_LIBS) -losg -losgDB
MACOSXLIBS = -L$(OSGHOME)/lib -losg -losgDB -lstdc++

C++FLAGS += -I. -I$(OSGHOME)/include 
LDFLAGS += -L$(OSGHOME)/lib

include $(OSGHOME)/Make/makerules

