TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	osgmovie.cpp\

LIBS     += -losgViewer -losgText -losgGA -losgDB -losgUtil -losg $(GL_LIBS) $(X_LIBS) $(OTHER_LIBS) 

INSTFILES = \
	$(CXXFILES)\
	GNUmakefile.inst=GNUmakefile

EXEC = osgmovie

INC +=  $(X_INC) 

include $(TOPDIR)/Make/makerules 

