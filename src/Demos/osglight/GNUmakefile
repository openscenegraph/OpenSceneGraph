TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	osglight.cpp\

LIBS     += $(OSG_LIBS) $(GLUT_LIB) $(GL_LIBS) $(X_LIBS) $(OTHER_LIBS) 

INSTFILES = \
	$(CXXFILES)\
	GNUmakefile.inst=GNUmakefile

EXEC = osglight

include $(TOPDIR)/Make/makerules 

