TOPDIR = ../../..
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

LIBS     += $(OSG_LIBS) $(GLUT_LIB) $(GL_LIBS) $(X_LIBS) $(OTHER_LIBS) 
#LIBS     += -losgProducer -l Producer $(OSG_LIBS) $(GL_LIBS) $(X_LIBS) $(OTHER_LIBS) 

INSTFILES = \
	$(CXXFILES)\
	$(HEADERFILES)\
	Makefile.inst=Makefile

EXEC = osghangglide

CXXFLAGS += -DUSE_GLUT
#CXXFLAGS += $(PRODUCER_INCLUDE_DIR)
#LDFLAGS  += $(PRODUCER_LIB_DIR)

include $(TOPDIR)/Make/makerules 
