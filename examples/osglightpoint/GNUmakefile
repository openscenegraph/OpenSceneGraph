TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	osglightpoint.cpp\

LIBS     += -losgProducer -lProducer -losgSim -losgText -losgGA -losgDB -losgUtil -losg $(GL_LIBS) $(X_LIBS) $(OTHER_LIBS) 

INSTFILES = \
	$(CXXFILES)\
	GNUmakefile.inst=GNUmakefile

EXEC = osglightpoint

INC +=  $(X_INC)

include $(TOPDIR)/Make/makerules 

