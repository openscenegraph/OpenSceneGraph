TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
        CameraGroup.cpp\
	EventAdapter.cpp\
        KeyboardMouseCallback.cpp\
        SceneHandler.cpp\
        Viewer.cpp\

LIBS += -lProducer $(GL_LIBS) -losgGA -losgUtil -losgDB -losg $(OTHER_LIBS)
DEF  += -DOSGPRODUCER_LIBRARY
TARGET_BASENAME = osgProducer
LIB = $(LIB_PREFIX)$(TARGET_BASENAME).$(LIB_EXT)

CXXFLAGS += $(OSG_INCLUDE_DIR)
LDFLAGS  += $(OSG_LIB_DIR)

include $(TOPDIR)/Make/makerules 
