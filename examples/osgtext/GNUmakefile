TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	osgtext.cpp\

LIBS     += -losgProducer -lProducer -losgText $(OSG_LIBS) $(GL_LIBS) $(X_LIBS) $(OTHER_LIBS) 

INSTFILES = \
	$(CXXFILES)\
	GNUmakefile.inst=GNUmakefile

EXEC = osgtext

INC += $(PRODUCER_INCLUDE_DIR)
LDFLAGS  += $(PRODUCER_LIB_DIR)

include $(TOPDIR)/Make/makerules 

