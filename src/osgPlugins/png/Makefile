#!smake
include $(OSGHOME)/Make/makedefs

C++FILES = \
        ReaderWriterPNG.cpp\



TARGET_BASENAME = osgdb_png

TARGET_LIB_FILES = lib$(TARGET_BASENAME).$(SO_EXT)

LIB = $(OSGHOME)/lib/osgPlugins/$(TARGET_BASENAME).$(SO_EXT)

TARGET_LOADER_FILES = osgPlugins/$(TARGET_BASENAME).$(SO_EXT)

LIBS    =  $(GL_LIBS) -losg -losgDB -lpng

C++FLAGS += -I. -I$(OSGHOME)/include 
LDFLAGS += -L$(OSGHOME)/lib

include $(OSGHOME)/Make/makerules
