TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	TestManipulator.cpp\
	osgimpostor.cpp\

LIBS     += $(OSG_LIBS) $(GLUT_LIB) $(GL_LIBS) $(X_LIBS) $(OTHER_LIBS) 

INSTFILES = \
	$(CXXFILES)\
	GNUmakefile.inst=GNUmakefile

EXEC = osgimpostor

include $(TOPDIR)/Make/makerules 
