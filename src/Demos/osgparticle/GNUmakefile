TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	osgparticle.cpp\

LIBS     += $(OSG_LIBS) $(GLUT_LIB) $(GL_LIBS) $(X_LIBS) $(OTHER_LIBS) -losgParticle

INSTFILES = \
	$(CXXFILES)\
	GNUmakefile.inst=GNUmakefile

EXEC = osgparticle

include $(TOPDIR)/Make/makerules 

