#!smake
include ../../../Make/makedefs

C++FILES = \
        ConvertFromPerformer.cpp\
        ConvertToPerformer.cpp\
        ReaderWriterPFB.cpp\

LIB = ../../../lib/osgPlugins/osgdb_pfb.so

#LIB = ../../../lib/libpfosg.so

#LIBS = -u __ucmpdi2 -lgcc -losg  ${PFLIBS} 
LIBS = -losg  ${PFLIBS} 

TARGET_LOADER_FILES = osgPlugins/osgdb_pfb.so

C++FLAGS += -I../../../include

LDFLAGS += -L../../../lib  -L/usr/X11R6/lib

include ../../../Make/makerules



