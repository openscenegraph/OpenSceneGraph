#!gmake
include ../../../Make/makedefs

C++FILES = \
        ReaderWriterQT.cpp\

CFILES = \
	QTtexture.c\

TARGET_BASENAME = osgdb_qt

LOADABLE = $(OSGHOME)/lib/osgPlugins/$(TARGET_BASENAME).$(DL_EXT)
LIB=

TARGET_LOADER_FILES = osgPlugins/$(TARGET_BASENAME).$(DL_EXT)

LIBS    =  $(GL_LIBS) -losg -losgDB
MACOSXLIBS = -framework QuickTime -L../../../lib -losg -losgUtil -losgDB -ldl -lm -lstdc++


C++FLAGS += -I. -I$(OSGHOME)/include 
LDFLAGS += -L$(OSGHOME)/lib

include $(OSGHOME)/Make/makerules

