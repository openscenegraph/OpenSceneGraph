TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
        DatabasePager.cpp\
	EventAdapter.cpp\
        KeyboardMouseCallback.cpp\
        OsgCameraGroup.cpp\
        OsgSceneHandler.cpp\
        ViewerEventHandler.cpp\
        Viewer.cpp\

LIBS += -lProducer $(GL_LIBS) -losgText -losgGA -losgUtil -losgDB -losg $(OTHER_LIBS)
DEF  += -DOSGPRODUCER_LIBRARY

TARGET_BASENAME = osgProducer
LIB = $(LIB_PREFIX)$(TARGET_BASENAME).$(LIB_EXT)

INC += $(PRODUCER_INCLUDE_DIR) -I/usr/X11R6/include
LDFLAGS  += $(PRODUCER_LIB_DIR)
    
ifeq ($(USE_OPEN_THREAD),1)
    DEF         += -D_USE_OPEN_THREAD
    INC         += $(INC_OPEN_THREAD)
    LDFLAGS     += $(LIB_OPEN_THREAD)
    LIBS        +=  -lOpenThread
endif

include $(TOPDIR)/Make/makerules 
