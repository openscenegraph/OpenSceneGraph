#!smake
include $(TOP)/Make/makedefs

C++FILES = \
	WXEventAdapter.cpp\
	Version.cpp\

TARGET_BASENAME = osgWX

LIBS    = -L$(TOP)/lib -losgDB -losgUtil -losg $(WXLIB) $(GL_LIBS)

LIB	= $(TOP)/lib/lib$(TARGET_BASENAME).$(SO_EXT)
#LIB	= $(TOP)/lib/lib$(TARGET_BASENAME).a

TARGET_LIB_FILES = lib$(TARGET_BASENAME).$(SO_EXT)
TARGET_INCLUDE_FILES = \
	osgWX/Export\
	osgWX/WXEventAdapter\
	osgWX/Version\

C++FLAGS += -I $(TOP)/include `wx-config --cflags`

include $(TOP)/Make/makerules

