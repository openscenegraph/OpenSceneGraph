TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	osgteapot.cpp\

LIBS     += -losgProducer -lProducer -losgText -losgGA -losgDB -losgUtil -losg $(GL_LIBS) $(X_LIBS) $(OTHER_LIBS) 

INSTFILES = \
	$(CXXFILES)\
	GNUmakefile.inst=GNUmakefile

EXEC = osgteapot

INC +=  $(X_INC)

include $(TOPDIR)/Make/makerules 
