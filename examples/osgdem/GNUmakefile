TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	osgdem.cpp\

LIBS     += -losgProducer -lProducer -losgFX -losgText -losgGA -losgDB -losgUtil -losg $(GL_LIBS) $(X_LIBS) $(OTHER_LIBS) 

INSTFILES = \
	$(CXXFILES)\
	GNUmakefile.inst=GNUmakefile

EXEC = osgdem

INC +=  $(X_INC) 

include $(TOPDIR)/Make/makerules 

