#!gmake
include ../../../Make/makedefs

CFILES = \
	stat.c\

C++FILES = \
        ConvertFromPerformer.cpp\
        ConvertToPerformer.cpp\
        ReaderWriterPFB.cpp\

LIB = ../../../lib/osgPlugins/osgdb_pfb.$(SO_EXT)

LIBS = -losg -losgDB \
	-lpf3ds\
	-lpfdem\
	-lpfdted\
	-lpfdwb\
	-lpfdxf\
	-lpfevt\
	-lpfflt\
	-lpfgds\
	-lpfgfo\
	-lpfim\
	-lpflsa\
	-lpflsb\
	-lpfnff\
	-lpfobj\
	-lpfpegg\
	-lpfpfb\
	-lpfphd\
	-lpfpts\
	-lpfrot\
	-lpfscale\
	-lpfsgf\
	-lpfsgo\
	-lpfsphere\
	-lpfsv\
	-lpftri\
	-lpfunc\
	-lpfvct\
	-lpfdu -lpfutil -lpf

# comment in for PFB loader texture support on IRIX
#   (note: you may need to build libimage.so from libimage.a)
# LIBS += -limage

LOADABLE = $(OSGHOME)/lib/osgPlugins/$(TARGET_BASENAME).$(DL_EXT)
LIB=

TARGET_LOADER_FILES = osgPlugins/osgdb_pfb.$(SO_EXT)

C++FLAGS += -g -I../../../include 

LDFLAGS += -g -L../../../lib  -L$(PFLOADERPATH)  $(LINKERARGS) -rpath $(PFLOADERPATH)

include ../../../Make/makerules


