TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	osgprecipitation.cpp\

LIBS     += -losgViewer -losgParticle -losgText -losgGA -losgDB -losgUtil -losg $(GL_LIBS) $(X_LIBS) $(OTHER_LIBS) 

INSTFILES = \
	$(CXXFILES)\
	GNUmakefile.inst=GNUmakefile

EXEC = osgprecipitation

INC +=  $(X_INC) 

include $(TOPDIR)/Make/makerules 

