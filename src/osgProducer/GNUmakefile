TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
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

include $(TOPDIR)/Make/makerules 
