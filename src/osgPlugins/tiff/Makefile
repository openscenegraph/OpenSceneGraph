#!smake
include ../../../Make/makedefs

C++FILES = \
        ReaderWriterTIFF.cpp\


TARGET_BASENAME = osgdb_tiff

TARGET_LIB_FILES = lib$(TARGET_BASENAME).$(SO_EXT)

LIB = $(OSGHOME)/lib/osgPlugins/$(TARGET_BASENAME).$(SO_EXT)

TARGET_LOADER_FILES = osgPlugins/$(TARGET_BASENAME).$(SO_EXT)

LIBS    =  $(GL_LIBS) -losg -losgDB -ltiff

C++FLAGS += -I. -I$(OSGHOME)/include 
LDFLAGS += -L$(OSGHOME)/lib

include $(OSGHOME)/Make/makerules

