TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	GLUTEventAdapter.cpp\
        Version.cpp\
        Window.cpp\
        Viewer.cpp\

LIBS += $(GLUT_LIB) -losgUtil -losgDB -losg $(OTHER_LIBS)
DEF  += -DOSGGLUT_LIBRARY
LIB = libosgGLUT.$(SO_EXT)

include $(TOPDIR)/Make/makerules 
