TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	osgviews.cpp\

LIBS     += $(OSG_LIBS) $(GLUT_LIB) $(GL_LIBS) $(X_LIBS) $(OTHER_LIBS) 

INSTFILES = \
	$(CXXFILES)\
	GNUmakefile.inst=GNUmakefile

EXEC = osgviews

include $(TOPDIR)/Make/makerules 
