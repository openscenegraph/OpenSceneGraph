#!gmake
SHELL   = /bin/sh

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

# override the compiler optimization, since GNU C++ 2.96 has problems with
# areas of the TXP loader code when compiling at -O2 optimization level.
# -O works fine?! Guess this is a tempory bug, or may be something more
# subtle in the TXP code itself, but my investigations havn't revealled
# anything to fix, right now it looks like just a compiler bug.
# Robert Osfield, Feb 2002.
CFLAGS    += -O

C++FLAGS += -I. -I$(OSGHOME)/include
LDFLAGS += -L$(OSGHOME)/lib

include $(OSGHOME)/Make/makerules

