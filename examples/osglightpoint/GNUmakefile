TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	osglightpoint.cpp\

LIBS     += -losgProducer -lProducer -losgSim -losgText -losgGA -losgDB -losgUtil -losg $(GL_LIBS) $(X_LIBS) $(OTHER_LIBS) 

INSTFILES = \
	$(CXXFILES)\
	GNUmakefile.inst=GNUmakefile

EXEC = osglightpoint

INC += $(PRODUCER_INCLUDE_DIR) -I/usr/X11R6/include
LDFLAGS  += $(PRODUCER_LIB_DIR)

include $(TOPDIR)/Make/makerules 

