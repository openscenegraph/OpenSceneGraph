#!smake
include $(OSGHOME)/Make/makedefs

C++FILES = ReaderWriterLWO.cpp lw.cpp

TARGET_BASENAME = osgdb_lwo

TARGET_LIB_FILES = lib$(TARGET_BASENAME).$(DL_EXT)

LOADABLE = $(OSGHOME)/lib/osgPlugins/$(TARGET_BASENAME).$(DL_EXT)
LIB=

TARGET_LOADER_FILES = osgPlugins/$(TARGET_BASENAME).$(DL_EXT)

LIBS    =  $(GL_LIBS) -losg -losgDB -losgUtil
MACOSXLIBS = -L$(OSGHOME)/lib -losg -losgDB -losgUtil -lstdc++

C++FLAGS += -I. -I$(OSGHOME)/include 
LDFLAGS += -L$(OSGHOME)/lib

include $(OSGHOME)/Make/makerules
