TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	osglogo.cpp\

LIBS     += -losgText $(OSG_LIBS) $(GLUT_LIB) $(GL_LIBS) $(X_LIBS) $(OTHER_LIBS) 

INSTFILES = \
	$(CXXFILES)\
	GNUmakefile.inst=GNUmakefile

EXEC = osglogo

include $(TOPDIR)/Make/makerules 
