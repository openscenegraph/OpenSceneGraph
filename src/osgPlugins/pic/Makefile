#!smake
include $(TOP)/Make/makedefs

C++FILES = \
        ReaderWriterPIC.cpp\


TARGET_BASENAME = osgdb_pic

TARGET_LIB_FILES = lib$(TARGET_BASENAME).$(SO_EXT)

LIB = $(TOP)/lib/osgPlugins/$(TARGET_BASENAME).$(SO_EXT)

TARGET_LOADER_FILES = osgPlugins/$(TARGET_BASENAME).$(SO_EXT)

LIBS    =  $(GL_LIBS) -losg -losgDB

C++FLAGS += -I. -I$(TOP)/include 
LDFLAGS += -L$(TOP)/lib

include $(TOP)/Make/makerules

