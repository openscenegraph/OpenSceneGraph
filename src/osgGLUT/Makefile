#!smake
include $(TOP)/Make/makedefs

C++FILES = \
	GLUTEventAdapter.cpp\
        Version.cpp\
        Window.cpp\
        Viewer.cpp\

TARGET_BASENAME = osgGLUT


LIBS    = -L$(TOP)/lib -losgDB -losgUtil -losg $(GLUTLIB) $(GL_LIBS)

LIB	= $(TOP)/lib/lib$(TARGET_BASENAME).$(SO_EXT)
#LIB	= $(TOP)/lib/lib$(TARGET_BASENAME).a

TARGET_LIB_FILES = lib$(TARGET_BASENAME).$(SO_EXT)
TARGET_INCLUDE_FILES = \
	osgGLUT/Export\
	osgGLUT/GLUTEventAdapter\
	osgGLUT/Version\
	osgGLUT/Window\
	osgGLUT/Viewer\
	osgGLUT/glut\
	

C++FLAGS += -I $(TOP)/include -DOSGGLUT_LIBRARY

include $(TOP)/Make/makerules

