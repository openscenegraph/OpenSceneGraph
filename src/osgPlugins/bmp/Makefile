#!smake
include $(OSGHOME)/Make/makedefs

C++FILES = \
        ReaderWriterBMP.cpp\

TARGET_BASENAME = osgdb_bmp

TARGET_LIB_FILES = lib$(TARGET_BASENAME).$(DL_EXT)

LIB=
LODABLE = $(OSGHOME)/lib/osgPlugins/$(TARGET_BASENAME).$(DL_EXT)

TARGET_LOADER_FILES = osgPlugins/$(TARGET_BASENAME).$(DL_EXT)

LIBS    =  $(GL_LIBS) -losg -losgDB
MACOSXLIBS = -L$(OSGOME)/lib -losg -losgDB -lstdc++

C++FLAGS += -I. -I$(OSGHOME)/include 
LDFLAGS += -L$(OSGHOME)/lib

include $(OSGHOME)/Make/makerules

