TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	osgcameragroup.cpp\

LIBS     += -losgProducer -lProducer -losgText -losgGA -losgDB -losgUtil -losg $(GL_LIBS) $(X_LIBS) $(OTHER_LIBS) 

INSTFILES = \
	$(CXXFILES)\
	GNUmakefile.inst=GNUmakefile

EXEC = osgcameragroup

INC += $(PRODUCER_INCLUDE_DIR) -I/usr/X11R6/include
LDFLAGS  += $(PRODUCER_LIB_DIR) -L/usr/X11R6/lib

include $(TOPDIR)/Make/makerules 

