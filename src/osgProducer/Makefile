TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	EventAdapter.cpp\
        KeyboardMouseCallback.cpp\

LIBS += -lProducer $(GL_LIBS) -losgGA -losgUtil -losgDB -losg $(OTHER_LIBS)
DEF  += -DOSGPRODUCER_LIBRARY
TARGET_BASENAME = osgProducer
LIB = $(LIB_PREFIX)$(TARGET_BASENAME).$(LIB_EXT)

include $(TOPDIR)/Make/makerules 
