#!smake
include $(OSGHOME)/Make/makedefs

C++FILES = \
	GLUTEventAdapter.cpp\
        Version.cpp\
        Window.cpp\
        Viewer.cpp\

TARGET_BASENAME = osgGLUT


LIBS    = -L$(OSGHOME)/lib -losgDB -losgUtil -losg $(GLUTLIB) $(GL_LIBS)
MACOSXLIBS = -L../../lib -losgDB -losgUtil -losg -ldl -lm -lstdc++


LIB	= $(OSGHOME)/lib/lib$(TARGET_BASENAME).$(SO_EXT)
#LIB	= $(OSGHOME)/lib/lib$(TARGET_BASENAME).a

TARGET_LIB_FILES = lib$(TARGET_BASENAME).$(SO_EXT)
TARGET_INCLUDE_FILES = \
	osgGLUT/Export\
	osgGLUT/GLUTEventAdapter\
	osgGLUT/Version\
	osgGLUT/Window\
	osgGLUT/Viewer\
	osgGLUT/glut\
	

C++FLAGS += -I $(OSGHOME)/include -DOSGGLUT_LIBRARY

include $(OSGHOME)/Make/makerules

