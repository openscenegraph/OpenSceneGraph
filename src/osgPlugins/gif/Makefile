#!smake
include $(OSGHOME)/Make/makedefs

C++FILES = \
        ReaderWriterGIF.cpp\

LIB = $(OSGHOME)/lib/osgPlugins/osgdb_gif.$(SO_EXT)

TARGET_LOADER_FILES = osgPlugins/osgdb_gif.$(SO_EXT)

LIBS = -lungif

C++FLAGS += -I. -I$(OSGHOME)/include  -I/usr/freeware/include
LDFLAGS += -L$(OSGHOME)/lib $(FREEBSD_LOCALLIBS) -L/usr/freeware/lib32

include $(OSGHOME)/Make/makerules



