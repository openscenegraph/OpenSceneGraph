TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	osganimate.cpp\

LIBS     += $(OSG_LIBS) $(GLUT_LIB) $(GL_LIBS) $(X_LIBS) $(OTHER_LIBS) 

INSTFILES = \
	$(CXXFILES)\
	GNUmakefile.inst=GNUmakefile

EXEC = osganimate

include $(TOPDIR)/Make/makerules 

