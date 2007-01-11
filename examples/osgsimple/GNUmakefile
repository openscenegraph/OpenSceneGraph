TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	osgsimple.cpp\

LIBS     += -losgViewer -losgGA -losgDB -losgUtil -losg $(GL_LIBS) $(X_LIBS) $(OTHER_LIBS) -lOpenThreads

INSTFILES = \
	$(CXXFILES)\
	GNUmakefile.inst=GNUmakefile

EXEC = osgsimple

include $(TOPDIR)/Make/makerules 

