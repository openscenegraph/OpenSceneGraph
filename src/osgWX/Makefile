#!smake
include $(OSGHOME)/Make/makedefs

C++FILES = \
	WXEventAdapter.cpp\
	Version.cpp\

TARGET_BASENAME = osgWX

LIBS    = -L$(OSGHOME)/lib -losgDB -losgUtil -losg $(WXLIB) $(GL_LIBS)

LIB	= $(OSGHOME)/lib/lib$(TARGET_BASENAME).$(SO_EXT)
#LIB	= $(OSGHOME)/lib/lib$(TARGET_BASENAME).a

TARGET_LIB_FILES = lib$(TARGET_BASENAME).$(SO_EXT)
TARGET_INCLUDE_FILES = \
	osgWX/Export\
	osgWX/WXEventAdapter\
	osgWX/Version\

C++FLAGS += -I $(OSGHOME)/include `wx-config --cflags`

include $(OSGHOME)/Make/makerules

