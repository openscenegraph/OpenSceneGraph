TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	osgreflect.cpp\

LIBS     += -losgProducer -lProducer $(OSG_LIBS) $(GL_LIBS) $(X_LIBS) $(OTHER_LIBS) 

INSTFILES = \
	$(CXXFILES)\
	Makefile.inst=Makefile

EXEC = osgreflect

INC += $(PRODUCER_INCLUDE_DIR)
LDFLAGS  += $(PRODUCER_LIB_DIR)

include $(TOPDIR)/Make/makerules 

