TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	osgviewer.cpp\

LIBS     += -losgProducer -lProducer $(OSG_LIBS) $(GL_LIBS) $(X_LIBS) $(OTHER_LIBS) 

INSTFILES = \
	$(CXXFILES)\
	Makefile.inst=Makefile

EXEC = osgviewer

INC += $(PRODUCER_INCLUDE_DIR)
LDFLAGS  += $(PRODUCER_LIB_DIR)

include $(TOPDIR)/Make/makerules 

