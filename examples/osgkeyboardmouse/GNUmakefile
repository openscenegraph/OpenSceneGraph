TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	osgkeyboardmouse.cpp\

LIBS     += -lProducer -losgFX -losgGA -losgDB -losgUtil -losg $(GL_LIBS) $(X_LIBS) $(OTHER_LIBS) -lOpenThreads

INSTFILES = \
	$(CXXFILES)\
	GNUmakefile.inst=GNUmakefile

EXEC = osgkeyboardmouse

INC      += $(X_INC) 
LDFLAGS  += -L/usr/X11R6/lib


ifeq ($(USE_OPEN_THREAD),1)
    DEF         += -D_USE_OPEN_THREAD
    INC         += $(INC_OPEN_THREAD)
    LDFLAGS     += $(LIB_OPEN_THREAD)
    LIBS        +=  -lOpenThread
endif

include $(TOPDIR)/Make/makerules 

