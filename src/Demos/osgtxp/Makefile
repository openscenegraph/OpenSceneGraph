TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	osgtxp.cpp\

LIBS     += $(OSG_LIBS) $(GLUT_LIB) $(GL_LIBS) $(X_LIBS) $(OTHER_LIBS) 

LIBS += -losgTXP

INSTFILES = \
	$(CXXFILES)\
	Makefile.inst=Makefile

EXEC = osgtxp

include $(TOPDIR)/Make/makerules 

