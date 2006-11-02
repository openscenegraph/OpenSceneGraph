TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	osgsimpleviewerProducer.cpp\

LIBS     += -lProducer -losgViewer -losgGA -losgDB -losgUtil -losg $(X_LIBS) $(OTHER_LIBS) -lOpenThreads

INSTFILES = \
	$(CXXFILES)\
	GNUmakefile.inst=GNUmakefile

EXEC = osgsimpleviewerProducer

INC      += $(X_INC) 
LDFLAGS  += -L/usr/X11R6/lib


ifeq ($(USE_OPEN_THREAD),1)
    DEF         += -D_USE_OPEN_THREAD
    INC         += $(INC_OPEN_THREAD)
    LDFLAGS     += $(LIB_OPEN_THREAD)
    LIBS        +=  -lOpenThread
endif

include $(TOPDIR)/Make/makerules 

