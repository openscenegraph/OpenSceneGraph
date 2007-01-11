TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	osgspacewarp.cpp\

LIBS     += -losgViewer -losgText -losgGA -losgDB -losgUtil -losg $(GL_LIBS) $(X_LIBS) $(OTHER_LIBS) 

INSTFILES = \
	$(CXXFILES)\
	GNUmakefile.inst=GNUmakefile

EXEC = osgspacewarp

INC +=  $(X_INC) 

include $(TOPDIR)/Make/makerules 

