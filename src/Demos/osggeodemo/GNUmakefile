TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	osggeodemo.cpp\

LIBS     += $(OSG_LIBS) $(GLUT_LIB) $(GL_LIBS) $(X_LIBS) $(OTHER_LIBS) 

INSTFILES = \
	$(CXXFILES)\
	GNUmakefile.inst=GNUmakefile

EXEC = osggeodemo

include $(TOPDIR)/Make/makerules 

