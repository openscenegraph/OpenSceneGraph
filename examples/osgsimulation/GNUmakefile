TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	osgsimulation.cpp\

LIBS     += -losgTerrain -losgFX -losgProducer -lProducer -losgParticle -losgSim -losgText -losgGA -losgDB -losgUtil -losg $(GDAL_LIBS) $(GL_LIBS) $(X_LIBS) $(OTHER_LIBS) 

INSTFILES = \
	$(CXXFILES)\
	GNUmakefile.inst=GNUmakefile

EXEC = osgsimulation

INC +=  $(X_INC) 

include $(TOPDIR)/Make/makerules 

