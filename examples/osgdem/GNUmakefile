TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	DataSet.cpp\
	osgdem.cpp\

LIBS     += -losgProducer -lProducer -losgTerrain -losgFX -losgGL2 -losgText -losgGA -losgDB -losgUtil -losg -lgdal $(GL_LIBS) $(X_LIBS) $(OTHER_LIBS) 

INSTFILES = \
	$(CXXFILES)\
	GNUmakefile.inst=GNUmakefile

EXEC = osgdem

INC +=  $(X_INC) 

include $(TOPDIR)/Make/makerules 

