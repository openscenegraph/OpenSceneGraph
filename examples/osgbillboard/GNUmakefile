TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	osgbillboard.cpp

LIBS     += -losgViewer -losgText -losgGA -losgDB -losgUtil -losg $(GL_LIBS) $(X_LIBS) $(OTHER_LIBS) 


INSTFILES = \
	$(CXXFILES)\
	GNUmakefile.inst=GNUmakefile

EXEC = osgbillboard

INC 	 +=  $(X_INC)

include $(TOPDIR)/Make/makerules 
