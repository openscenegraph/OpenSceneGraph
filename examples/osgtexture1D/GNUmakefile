TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	osgtexture1D.cpp\

LIBS     += -losgViewer -losgText -losgGA -losgDB -losgUtil -losg $(GL_LIBS) $(X_LIBS) $(OTHER_LIBS) 

INSTFILES = \
	$(CXXFILES)\
	GNUmakefile.inst=GNUmakefile

EXEC = osgtexture1D

INC +=  $(X_INC)

include $(TOPDIR)/Make/makerules 
