TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	osggeometry.cpp\

LIBS     += $(OSG_LIBS) $(GLUT_LIB) $(GL_LIBS) $(X_LIBS) $(OTHER_LIBS) 

INSTFILES = \
	$(CXXFILES)\
	Makefile.inst=Makefile

EXEC = osggeometry

include $(TOPDIR)/Make/makerules 
