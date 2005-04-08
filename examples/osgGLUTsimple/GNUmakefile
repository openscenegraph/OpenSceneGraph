TOPDIR = ../..
include $(TOPDIR)/Make/makedefs

CXXFILES =\
	osgGLUTsimple.cpp\

LIBS     += -losgDB -losgUtil -losg -lglut $(GL_LIBS) $(X_LIBS) $(OTHER_LIBS)

INSTFILES = \
	$(CXXFILES)\
	GNUmakefile.inst=GNUmakefile

EXEC = osgGLUTsimple

INC      += $(X_INC) 
LDFLAGS  += -L/usr/X11R6/lib

include $(TOPDIR)/Make/makerules 
