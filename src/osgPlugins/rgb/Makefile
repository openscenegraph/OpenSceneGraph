#!gmake
include $(OSGHOME)/Make/makedefs

C++FILES = \
        ReaderWriterRGB.cpp\

TARGET_BASENAME = osgdb_rgb

LOADABLE = $(OSGHOME)/lib/osgPlugins/$(TARGET_BASENAME).$(DL_EXT)
LIB=

TARGET_LOADER_FILES = osgPlugins/$(TARGET_BASENAME).$(DL_EXT)

LIBS    =  $(GL_LIBS) -losg -losgDB
MACOSXLIBS = -L../../../lib -losg -losgUtil -losgDB -ldl -lstdc++

C++FLAGS += -I. -I$(OSGHOME)/include 
LDFLAGS += -L$(OSGHOME)/lib

include $(OSGHOME)/Make/makerules



