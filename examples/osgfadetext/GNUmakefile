TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	osgfadetext.cpp\

LIBS     += -losgTerrain -losgFX -losgViewer -losgSim -losgText -losgGA -losgDB -losgUtil -losg $(GDAL_LIBS) $(GL_LIBS) $(X_LIBS) $(OTHER_LIBS) 

INSTFILES = \
	$(CXXFILES)\
	GNUmakefile.inst=GNUmakefile

EXEC = osgfadetext

INC +=  $(X_INC) 

include $(TOPDIR)/Make/makerules 

