#!smake
include ../../Make/makedefs

C++FILES = \
	GLUTEventAdapter.cpp\
        Version.cpp\
        Viewer.cpp\

TARGET_BASENAME = osgGLUT


LIBS    = -ldl

LIB	= ../../lib/lib$(TARGET_BASENAME).so
#LIB	= ../../lib/lib$(TARGET_BASENAME).a

TARGET_LIB_FILES = lib$(TARGET_BASENAME).so
TARGET_INCLUDE_FILES = \
	osgGLUT/Export\
	osgGLUT/Viewer\
	osgGLUT/GLUTEventAdapter\

C++FLAGS += -I ../../include

include ../../Make/makerules

