TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	osgscribe.cpp\

LIBS     += -losgProducer -lProducer -losgText -losgGA -losgDB -losgUtil -losg $(GL_LIBS) $(X_LIBS) $(OTHER_LIBS) 

INSTFILES = \
	$(CXXFILES)\
	GNUmakefile.inst=GNUmakefile

EXEC = osgscribe

INC +=  $(X_INC)

include $(TOPDIR)/Make/makerules 
