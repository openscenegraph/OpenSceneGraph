TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	osgshadow.cpp\

LIBS     += -losgViewer -losgShadow -losgText -losgGA -losgDB -losgUtil -losg $(GL_LIBS) $(X_LIBS) $(OTHER_LIBS) 

INSTFILES = \
	$(CXXFILES)\
	GNUmakefile.inst=GNUmakefile

EXEC = osgshadow

INC +=  $(X_INC)

include $(TOPDIR)/Make/makerules 
