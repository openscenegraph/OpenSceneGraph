TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	osglightpoint.cpp\

LIBS     += -losgSim $(OSG_LIBS) $(GLUT_LIB) $(GL_LIBS) $(X_LIBS) $(OTHER_LIBS) 

INSTFILES = \
	$(CXXFILES)\
	Makefile.inst=Makefile

EXEC = osglightpoint

include $(TOPDIR)/Make/makerules 

