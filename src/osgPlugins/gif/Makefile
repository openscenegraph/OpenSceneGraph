#!smake
include $(OSGHOME)/Make/makedefs

C++FILES = \
        ReaderWriterGIF.cpp\

TARGET_BASENAME = osgdb_gif

TARGET_LIB_FILES = lib$(TARGET_BASENAME).$(DL_EXT)

LIB=
LOADABLE = $(OSGHOME)/lib/osgPlugins/$(TARGET_BASENAME).$(DL_EXT)

TARGET_LOADER_FILES = osgPlugins/$(TARGET_BASENAME).$(DL_EXT)

LIBS = $(GL_LIBS) -losg -losgDB -lungif 
MACOSXLIBS = -L$(OSGHOME)/lib -losg -losgDB -lungif -lstdc++

C++FLAGS += -I. -I$(OSGHOME)/include  -I/usr/freeware/include
LDFLAGS += -L$(OSGHOME)/lib $(FREEBSD_LOCALLIBS) -L/usr/freeware/lib32

include $(OSGHOME)/Make/makerules



