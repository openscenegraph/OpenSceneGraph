TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	GL2Scene.cpp \
	Noise.cpp \
	osgshaders.cpp

LIBS     += -losgProducer -lProducer -losgText -losgGA -losgDB -losgUtil -losg $(GL_LIBS) $(X_LIBS) $(OTHER_LIBS) 

INSTFILES = \
	$(CXXFILES)\
	GNUmakefile.inst=GNUmakefile

EXEC = osgshaders

INC +=  $(X_INC) 

include $(TOPDIR)/Make/makerules 

