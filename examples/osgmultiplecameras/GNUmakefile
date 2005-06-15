TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	osgmultiplecameras.cpp\

LIBS     += -losgProducer -lProducer -losgText -losgProducer -lProducer -losgGA -losgDB -losgUtil -losg $(GL_LIBS) $(X_LIBS) $(OTHER_LIBS) 

INSTFILES = \
	$(CXXFILES)\
	GNUmakefile.inst=GNUmakefile

EXEC = osgmultiplecameras

INC +=  $(X_INC)

include $(TOPDIR)/Make/makerules 

