TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	osgdemeter.cpp\

LIBS     +=  -lDemeterOSG -lDemeter -losgProducer -lProducer -losgText -losgGA -losgDB -losgUtil -losg $(GL_LIBS) $(X_LIBS) $(OTHER_LIBS) 

INSTFILES = \
	$(CXXFILES)\
	GNUmakefile.inst=GNUmakefile

EXEC = osgdemeter

INC +=  -I/usr/X11R6/include 

include $(TOPDIR)/Make/makerules 

