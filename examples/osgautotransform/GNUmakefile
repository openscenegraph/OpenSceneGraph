TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	osgautotransform.cpp\

LIBS     += -losgProducer -lProducer -losgText -losgProducer -lProducer -losgGA -losgDB -losgUtil -losg $(GL_LIBS) $(X_LIBS) $(OTHER_LIBS) 

INSTFILES = \
	$(CXXFILES)\
	GNUmakefile.inst=GNUmakefile

EXEC = osgautotransform

INC 	 +=  -I/usr/X11R6/include

include $(TOPDIR)/Make/makerules 

