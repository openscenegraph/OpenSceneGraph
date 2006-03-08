TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
        KeyboardMouseCallback.cpp\
        GraphicsContextImplementation.cpp\
        OsgCameraGroup.cpp\
        OsgSceneHandler.cpp\
        ViewerEventHandler.cpp\
        Viewer.cpp\
        Version.cpp\

LIBS += -lProducer $(GL_LIBS) -losgText -losgGA -losgUtil -losgDB -losg $(OTHER_LIBS)
DEF  += -DOSGPRODUCER_LIBRARY

TARGET_BASENAME = osgProducer
LIB = $(LIB_PREFIX)$(TARGET_BASENAME).$(LIB_EXT)

INC += $(X_INC)
    
include $(TOPDIR)/Make/makerules 
