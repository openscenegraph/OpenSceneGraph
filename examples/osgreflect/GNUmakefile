TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	osgreflect.cpp\

LIBS     += -losgProducer -lProducer -losgText -losgGA -losgDB -losgUtil -losg $(GL_LIBS) $(X_LIBS) $(OTHER_LIBS) 

INSTFILES = \
	$(CXXFILES)\
	GNUmakefile.inst=GNUmakefile

EXEC = osgreflect

INC +=  $(X_INC)

include $(TOPDIR)/Make/makerules 

