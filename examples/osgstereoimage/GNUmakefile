TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	osgstereoimage.cpp\

LIBS     += -losgProducer -lProducer -losgText -losgDB -losgUtil -losg $(GL_LIBS) $(X_LIBS) $(OTHER_LIBS) 

INSTFILES = \
	$(CXXFILES)\
	GNUmakefile.inst=GNUmakefile

EXEC = osgstereoimage

INC += $(PRODUCER_INCLUDE_DIR)
LDFLAGS  += $(PRODUCER_LIB_DIR)

include $(TOPDIR)/Make/makerules 

