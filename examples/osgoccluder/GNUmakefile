TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	osgoccluder.cpp\

LIBS     += -losgProducer -lProducer -losgText -losgGA -losgDB -losgUtil -losg $(GL_LIBS) $(X_LIBS) $(OTHER_LIBS) 

INSTFILES = \
	$(CXXFILES)\
	GNUmakefile.inst=GNUmakefile

EXEC = osgoccluder

INC +=  $(X_INC)

include $(TOPDIR)/Make/makerules 

