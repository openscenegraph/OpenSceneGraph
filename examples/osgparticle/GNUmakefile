TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	osgparticle.cpp\

LIBS     += -losgProducer -lProducer -losgDB -losgUtil -losg $(GL_LIBS) $(X_LIBS) $(OTHER_LIBS) -losgParticle

INSTFILES = \
	$(CXXFILES)\
	GNUmakefile.inst=GNUmakefile

EXEC = osgparticle

INC += $(PRODUCER_INCLUDE_DIR)
LDFLAGS  += $(PRODUCER_LIB_DIR)

include $(TOPDIR)/Make/makerules 

