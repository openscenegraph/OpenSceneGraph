#!smake
include ../../../Make/makedefs

C++FILES = \
        ReaderWriterTIFF.cpp\


TARGET_BASENAME = osgdb_tiff

TARGET_LIB_FILES = lib$(TARGET_BASENAME).$(DL_EXT)

LIB=
LOADABLE = $(OSGHOME)/lib/osgPlugins/$(TARGET_BASENAME).$(DL_EXT)

TARGET_LOADER_FILES = osgPlugins/$(TARGET_BASENAME).$(DL_EXT)

LIBS = $(GL_LIBS) -losg -losgDB -ltiff 
MACOSXLIBS = -L$(OSGHOME)/lib -losg -losgDB -ltiff -lstdc++

C++FLAGS += -I. -I$(OSGHOME)/include 
LDFLAGS += -L$(OSGHOME)/lib

include $(OSGHOME)/Make/makerules

