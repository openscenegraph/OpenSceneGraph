TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 


CXXFILES = \
    DefaultFont.cpp\
    Font.cpp\
    Text.cpp\
    Version.cpp\

DEF     += -DOSGTEXT_LIBRARY

LIBS  +=  -losgDB -losgUtil -losg $(GL_LIBS) $(OTHER_LIBS)

TARGET_BASENAME = osgText
LIB = $(LIB_PREFIX)$(TARGET_BASENAME).$(LIB_EXT)

include $(TOPDIR)/Make/makerules 
