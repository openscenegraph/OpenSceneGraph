TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 


CXXFILES = \
	Extensions.cpp \
	Version.cpp \
	ProgramObject.cpp


DEF     += -DOSGGL2_LIBRARY

LIBS  += -losg -losgUtil $(GL_LIBS) $(OTHER_LIBS)

TARGET_BASENAME = osgGL2
LIB = $(LIB_PREFIX)$(TARGET_BASENAME).$(LIB_EXT)

include $(TOPDIR)/Make/makerules 


