TOPDIR = ../..
include $(TOPDIR)/Make/makedefs

CXXFILES =\
	osgsimpleviewerFLTK.cpp\

LIBS     += -losgViewer -losgText -losgGA -losgDB -losgUtil -losg -lfltk -lfltk_gl $(GL_LIBS) $(OTHER_LIBS)

INSTFILES = \
	$(CXXFILES)\
	GNUmakefile.inst=GNUmakefile

EXEC = osgsimpleviewerFLTK


include $(TOPDIR)/Make/makerules 
