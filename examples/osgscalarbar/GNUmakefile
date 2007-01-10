TOPDIR = ../..
include $(TOPDIR)/Make/makedefs

CXXFILES =\
	osgscalarbar.cpp\

LIBS     += -losgText -losgViewer -losgSim -losgText -losgGA -losgDB -losgUtil -losg $(GL_LIBS) $(X_LIBS) $(OTHER_LIBS)

INSTFILES = \
	$(CXXFILES)\
	GNUmakefile.inst=GNUmakefile

EXEC = osgscalarbar

INC += $(X_INC)

include $(TOPDIR)/Make/makerules 
