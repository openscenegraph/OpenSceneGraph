TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	osghud.cpp\

LIBS     += -losgProducer -lProducer -losgText $(OSG_LIBS) $(GL_LIBS) $(X_LIBS) $(OTHER_LIBS) 

INSTFILES = \
	$(CXXFILES)\
	Makefile.inst=Makefile

EXEC = osghud

INC += $(PRODUCER_INCLUDE_DIR)
LDFLAGS  += $(PRODUCER_LIB_DIR)

include $(TOPDIR)/Make/makerules 

