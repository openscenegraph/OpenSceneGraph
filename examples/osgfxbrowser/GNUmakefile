TOPDIR = ../..
include $(TOPDIR)/Make/makedefs

CXXFILES =\
	osgfxbrowser.cpp\
	Frame.cpp\

LIBS     += -losgProducer -lProducer -losgText -losgGA -losgDB -losgUtil -losg -losgFX $(GL_LIBS) $(X_LIBS) $(OTHER_LIBS)

INSTFILES = \
	$(CXXFILES)\
	GNUmakefile.inst=GNUmakefile

EXEC = osgfxbrowser

INC +=  $(X_INC)

include $(TOPDIR)/Make/makerules
