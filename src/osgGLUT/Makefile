TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	GLUTEventAdapter.cpp\
        Version.cpp\
        Window.cpp\
        Viewer.cpp\

LIBS += $(GLUT_LIB) $(GL_LIBS) -losgGA -losgUtil -losgDB -losg $(OTHER_LIBS)
DEF  += -DOSGGLUT_LIBRARY
TARGET_BASENAME = osgGLUT
LIB = $(LIB_PREFIX)$(TARGET_BASENAME).$(LIB_EXT)

include $(TOPDIR)/Make/makerules 
