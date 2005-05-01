TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	TestManipulator.cpp\
	osgimpostor.cpp\

LIBS     += -losgProducer -lProducer -losgText -losgSim -losgGA -losgDB -losgUtil -losg $(GL_LIBS) $(X_LIBS) $(OTHER_LIBS) 

INSTFILES = \
	$(CXXFILES)\
	GNUmakefile.inst=GNUmakefile

EXEC = osgimpostor

INC +=  $(X_INC)

include $(TOPDIR)/Make/makerules 
