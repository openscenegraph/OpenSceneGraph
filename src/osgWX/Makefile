#!smake
include ../../Make/makedefs

C++FILES = \
	WXEventAdapter.cpp\
        Version.cpp\

TARGET_BASENAME = osgWX

LIBS    = -L../../lib -losgDB -losgUtil -losg $(WXLIB) -lGLU -lGL -lm

LIB	= ../../lib/lib$(TARGET_BASENAME).so
#LIB	= ../../lib/lib$(TARGET_BASENAME).a

TARGET_LIB_FILES = lib$(TARGET_BASENAME).so
TARGET_INCLUDE_FILES = \
	osgWX/Export\
	osgWX/WXEventAdapter\
	osgWX/Version\

C++FLAGS += -I ../../include `wx-config --cflags`

include ../../Make/makerules

