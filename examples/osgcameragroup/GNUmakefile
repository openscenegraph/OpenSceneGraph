TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	osgcameragroup.cpp\

LIBS     += -losgProducer -lProducer -losgText -losgGA -losgDB -losgUtil -losg $(GL_LIBS) $(X_LIBS) $(OTHER_LIBS) 

INSTFILES = \
	$(CXXFILES)\
	GNUmakefile.inst=GNUmakefile

EXEC = osgcameragroup

INC += $(PRODUCER_INCLUDE_DIR) -I/usr/X11R6/include -I/usr/X11R6/include
LDFLAGS  += $(PRODUCER_LIB_DIR) -L/usr/X11R6/lib


ifeq ($(USE_OPEN_THREAD),1)
    DEF         += -D_USE_OPEN_THREAD
    INC         += $(INC_OPEN_THREAD)
    LDFLAGS     += $(LIB_OPEN_THREAD)
    LIBS        +=  -lOpenThread
endif

include $(TOPDIR)/Make/makerules 

