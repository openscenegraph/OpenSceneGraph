TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	base.cpp\
	GliderManipulator.cpp\
	hangglide.cpp\
	hat.cpp\
	ReaderWriterFLY.cpp\
	sky.cpp\
	tank.cpp\
	terrain.cpp\
	trees.cpp\

HEADERFILES = \
	GliderManipulator.h\
	hat.h\
	terrain_data.h\

LIBS     += $(OSG_LIBS) $(GLUT_LIB) $(GL_LIBS) $(X_LIBS) $(OTHER_LIBS) 

INSTFILES = \
	$(CXXFILES)\
	$(HEADERFILES)\
	Makefile.inst=Makefile

EXEC = osghangglide

include $(TOPDIR)/Make/makerules 
