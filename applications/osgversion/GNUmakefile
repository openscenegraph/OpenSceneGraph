TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	osgversion.cpp\

LIBS     += -losg $(GL_LIBS) $(X_LIBS) $(OTHER_LIBS) 

INSTFILES = \
	$(CXXFILES)\
	GNUmakefile.inst=GNUmakefile

EXEC = osgversion

INC +=  $(X_INC)

include $(TOPDIR)/Make/makerules 
