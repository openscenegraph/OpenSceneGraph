TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	CreateShadowedScene.cpp\
	osgshadowtexture.cpp\

LIBS     += $(OSG_LIBS) $(GLUT_LIB) $(GL_LIBS) $(X_LIBS) $(OTHER_LIBS) 

INSTFILES = \
	$(CXXFILES)\
	Makefile.inst=Makefile

EXEC = osgshadowtexture

include $(TOPDIR)/Make/makerules 
