#!smake
include ../../Make/makedefs

C++FILES = \
	GLUTEventAdapter.cpp\
        Version.cpp\
        Window.cpp\
        Viewer.cpp\

TARGET_BASENAME = osgGLUT


LIBS    = -L../../lib -losgDB -losgUtil -losg $(GLUTLIB) -lGLU -lGL -lm

LIB	= ../../lib/lib$(TARGET_BASENAME).so
#LIB	= ../../lib/lib$(TARGET_BASENAME).a

TARGET_LIB_FILES = lib$(TARGET_BASENAME).so
TARGET_INCLUDE_FILES = \
	osgGLUT/Export\
	osgGLUT/GLUTEventAdapter\
	osgGLUT/Version\
	osgGLUT/Window\
	osgGLUT/Viewer\

C++FLAGS += -I ../../include

include ../../Make/makerules

