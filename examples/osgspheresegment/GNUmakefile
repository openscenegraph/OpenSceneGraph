TOPDIR = ../..
include $(TOPDIR)/Make/makedefs

CXXFILES =\
	osgspheresegment.cpp\

LIBS     += -losgProducer -lProducer -losgParticle -losgSim -losgText -losgGA -losgDB -losgUtil -losg $(GL_LIBS) $(X_LIBS) $(OTHER_LIBS)

INSTFILES = \
	$(CXXFILES)\
	GNUmakefile.inst=GNUmakefile

EXEC = osgspheresegment

INC += $(X_INC)

include $(TOPDIR)/Make/makerules 
