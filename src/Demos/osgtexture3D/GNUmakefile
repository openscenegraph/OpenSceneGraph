TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	osgtexture3D.cpp\

LIBS     += $(OSG_LIBS) $(GLUT_LIB) $(GL_LIBS) $(X_LIBS) $(OTHER_LIBS) 

INSTFILES = \
	$(CXXFILES)\
	GNUmakefile.inst=GNUmakefile

EXEC = osgtexture3D

include $(TOPDIR)/Make/makerules 
