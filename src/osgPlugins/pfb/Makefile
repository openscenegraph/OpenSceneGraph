#!smake
include ../../../Make/makedefs

CFILES = \
	stat.c\

C++FILES = \
        ConvertFromPerformer.cpp\
        ConvertToPerformer.cpp\
        ReaderWriterPFB.cpp\

LIB = ../../../lib/osgPlugins/osgdb_pfb.so

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

TARGET_LOADER_FILES = osgPlugins/osgdb_pfb.so

C++FLAGS += -g -I../../../include 

LDFLAGS += -g -L../../../lib  -L$(PFLOADERPATH)  -Xlinker -rpath $(PFLOADERPATH)

include ../../../Make/makerules


