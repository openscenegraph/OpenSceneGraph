TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	base.cpp\
	GliderManipulator.cpp\
	osghangglide.cpp\
	hat.cpp\
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

LIBS     += -losgProducer -lProducer -losgText -losgGA -losgDB -losgUtil -losg $(GL_LIBS) $(X_LIBS) $(OTHER_LIBS) 

INSTFILES = \
	$(CXXFILES)\
	$(HEADERFILES)\
	GNUmakefile.inst=GNUmakefile

EXEC = osghangglide

INC +=  $(X_INC)

include $(TOPDIR)/Make/makerules 
