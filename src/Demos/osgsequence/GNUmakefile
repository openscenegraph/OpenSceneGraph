TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	osgsequence.cpp\

LIBS     += $(OSG_LIBS) $(GLUT_LIB) $(GL_LIBS) $(X_LIBS) $(OTHER_LIBS) 

INSTFILES = \
	$(CXXFILES)\
	GNUmakefile.inst=GNUmakefile

EXEC = osgsequence

include $(TOPDIR)/Make/makerules 
