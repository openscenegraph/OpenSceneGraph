TOPDIR = ../..
include $(TOPDIR)/Make/makedefs

CXXFILES =\
	osgGLUTkeyboardmouse.cpp\

LIBS     += -losgDB -losgUtil -losg $(GLUT_LIBS) $(GL_LIBS) $(X_LIBS) $(OTHER_LIBS)

INSTFILES = \
	$(CXXFILES)\
	GNUmakefile.inst=GNUmakefile

EXEC = osgGLUTkeyboardmouse

INC      += $(X_INC) 
LDFLAGS  += -L/usr/X11R6/lib

include $(TOPDIR)/Make/makerules 
