TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	osgparticle.cpp\

LIBS     += -losgProducer -lProducer -losgText -losgGA -losgDB -losgUtil -losg $(GL_LIBS) $(X_LIBS) $(OTHER_LIBS) -losgParticle

INSTFILES = \
	$(CXXFILES)\
	GNUmakefile.inst=GNUmakefile

EXEC = osgparticle

INC +=  $(X_INC)

include $(TOPDIR)/Make/makerules 

