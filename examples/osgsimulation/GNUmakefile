TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	osgsimulation.cpp\

LIBS     += -losgViewer -losgTerrain -losgFX -losgParticle -losgSim -losgText -losgGA -losgDB -losgUtil -losg $(GDAL_LIBS) $(GL_LIBS) $(X_LIBS) $(OTHER_LIBS) 

INSTFILES = \
	$(CXXFILES)\
	GNUmakefile.inst=GNUmakefile

EXEC = osgsimulation

INC +=  $(X_INC) 

include $(TOPDIR)/Make/makerules 

