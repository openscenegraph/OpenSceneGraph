TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	base.cpp\
	GliderManipulator.cpp\
	osghangglide.cpp\
	hat.cpp\
	ReaderWriterFLY.cpp\
	sky.cpp\
	tank.cpp\
	terrain.cpp\
	trees.cpp\

HEADERFILES = \
	GliderManipulator.h\
	hat.h\
	terrain_coords.h\
	terrain_normals.h\
	terrain_texcoords.h\

LIBS     += $(OSG_LIBS) $(GLUT_LIB) $(GL_LIBS) $(X_LIBS) $(OTHER_LIBS) 

INSTFILES = \
	$(CXXFILES)\
	$(HEADERFILES)\
	Makefile.inst=Makefile

EXEC = osghangglide

include $(TOPDIR)/Make/makerules 
