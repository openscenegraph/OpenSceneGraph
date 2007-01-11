TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	osgslice.cpp\

LIBS     += -losgViewer -losgDB -losgUtil -losg $(GL_LIBS) $(X_LIBS) $(OTHER_LIBS) -lOpenThreads

INSTFILES = \
	$(CXXFILES)\
	GNUmakefile.inst=GNUmakefile

EXEC = osgslice

INC      += $(X_INC) 
LDFLAGS  += -L/usr/X11R6/lib

include $(TOPDIR)/Make/makerules 

