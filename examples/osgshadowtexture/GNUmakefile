TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	CreateShadowedScene.cpp\
	osgshadowtexture.cpp\

LIBS     += -losgProducer -lProducer -losgText -losgGA -losgDB -losgUtil -losg $(GL_LIBS) $(X_LIBS) $(OTHER_LIBS) 

INSTFILES = \
	$(CXXFILES)\
	GNUmakefile.inst=GNUmakefile

EXEC = osgshadowtexture

INC +=  $(X_INC)

include $(TOPDIR)/Make/makerules 
