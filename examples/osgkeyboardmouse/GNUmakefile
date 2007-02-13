TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	osgkeyboardmouse.cpp\

LIBS     += -losgViewer -losgText -losgFX -losgGA -losgDB -losgUtil -losg $(X_LIBS) $(OTHER_LIBS) -lOpenThreads

INSTFILES = \
	$(CXXFILES)\
	GNUmakefile.inst=GNUmakefile

EXEC = osgkeyboardmouse

include $(TOPDIR)/Make/makerules 

