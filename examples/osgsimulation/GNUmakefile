TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	osgsimulation.cpp\

LIBS     += -losgTerrain -losgProducer -lProducer -losgParticle -losgSim -losgText -losgGA -losgDB -losgUtil -losg $(GL_LIBS) $(X_LIBS) $(OTHER_LIBS) 

INSTFILES = \
	$(CXXFILES)\
	GNUmakefile.inst=GNUmakefile

EXEC = osgsimulation

INC +=  $(X_INC) 

include $(TOPDIR)/Make/makerules 

