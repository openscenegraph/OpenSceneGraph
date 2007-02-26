TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	osgmanipulator.cpp\

LIBS     += -losgViewer -losgManipulator -losgText -losgGA -losgDB -losgUtil -losg $(GL_LIBS) $(X_LIBS) $(OTHER_LIBS) 

INSTFILES = \
	$(CXXFILES)\
	GNUmakefile.inst=GNUmakefile

EXEC = osgmanipulator

INC +=  $(X_INC) 

include $(TOPDIR)/Make/makerules 

