#!smake
include $(OSGHOME)/Make/makedefs

C++FILES = \
        ReaderWriterPNG.cpp\



TARGET_BASENAME = osgdb_png

LIB=
LOADABLE = $(OSGHOME)/lib/osgPlugins/$(TARGET_BASENAME).$(DL_EXT)

TARGET_LOADER_FILES = osgPlugins/$(TARGET_BASENAME).$(DL_EXT)

LIBS = $(GL_LIBS) -losg -losgDB -lpng 
MACOSXLIBS = -L$(OSGHOME)/lib -losg -losgDB -lpng -lstdc++


C++FLAGS += -I. -I$(OSGHOME)/include 
LDFLAGS += -L$(OSGHOME)/lib

include $(OSGHOME)/Make/makerules
