TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	TestManipulator.cpp\
	osgimpostor.cpp\

LIBS     += -losgProducer -lProducer -losgDB -losgUtil -losg $(GL_LIBS) $(X_LIBS) $(OTHER_LIBS) 

INSTFILES = \
	$(CXXFILES)\
	GNUmakefile.inst=GNUmakefile

EXEC = osgimpostor

INC += $(PRODUCER_INCLUDE_DIR)
LDFLAGS  += $(PRODUCER_LIB_DIR)

include $(TOPDIR)/Make/makerules 
