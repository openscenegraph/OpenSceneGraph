TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	osgintersection.cpp\

LIBS     += -losgText -losgGA -losgDB -losgUtil -losg $(GL_LIBS) $(X_LIBS) $(OTHER_LIBS) 

INSTFILES = \
	$(CXXFILES)\
	GNUmakefile.inst=GNUmakefile

EXEC = osgintersection

INC +=  $(X_INC) 

include $(TOPDIR)/Make/makerules 

