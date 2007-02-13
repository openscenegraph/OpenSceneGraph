TOPDIR = ../..
include $(TOPDIR)/Make/makedefs

CXXFILES =\
	osgsimpleviewerGLUT.cpp\

LIBS     += -losgViewer -losgText -losgGA -losgDB -losgUtil -losg $(GLUT_LIBS) $(GL_LIBS) $(X_LIBS) $(OTHER_LIBS)

INSTFILES = \
	$(CXXFILES)\
	GNUmakefile.inst=GNUmakefile

EXEC = osgsimpleviewerGLUT

INC      += $(X_INC) 
LDFLAGS  += -L/usr/X11R6/lib

include $(TOPDIR)/Make/makerules 
