TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	osgarchive.cpp\

LIBS     += -losgDB -losgUtil -losg $(GL_LIBS) $(X_LIBS) $(OTHER_LIBS) 

INSTFILES = \
	$(CXXFILES)\
	GNUmakefile.inst=GNUmakefile

EXEC = osgarchive

INC +=  $(X_INC) 

include $(TOPDIR)/Make/makerules 

