TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	osggeodemo.cpp\

LIBS     += $(OSG_LIBS) $(GLUT_LIB) $(GL_LIBS) $(X_LIBS) $(OTHER_LIBS) 

INSTFILES = \
	$(CXXFILES)\
	Makefile.inst=Makefile

EXEC = osggeodemo

include $(TOPDIR)/Make/makerules 

