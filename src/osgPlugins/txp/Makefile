#!smake
SHELL   = /bin/sh
OSGHOME = $(shell cd ../../..; pwd)

include $(OSGHOME)/Make/makedefs

C++FILES = \
	ReaderWriterTXP.cpp\
	TrPageArchive.cpp\
	TrPageParser.cpp\
	trpage_basic.cpp\
	trpage_geom.cpp\
	trpage_header.cpp\
	trpage_tile.cpp\
	trpage_readbuf.cpp\
	trpage_rarchive.cpp\
	trpage_writebuf.cpp\
	trpage_warchive.cpp\
	trpage_parse.cpp\
	trpage_nodes.cpp\
	trpage_model.cpp\
	trpage_material.cpp\
	trpage_swap.cpp

TARGET_BASENAME = osgdb_txp

LIB = 
LOADABLE = $(OSGHOME)/lib/osgPlugins/$(TARGET_BASENAME).$(DL_EXT)

TARGET_LOADER_FILES = osgPlugins/$(TARGET_BASENAME).$(DL_EXT)

LIBS    =  $(GL_LIBS) -losg -losgDB
MACOSXLIBS = -L$(OSGHOME)/lib -losg -losgDB -lstdc++

C++FLAGS += -I. -I$(OSGHOME)/include
LDFLAGS += -L$(OSGHOME)/lib

include $(OSGHOME)/Make/makerules

