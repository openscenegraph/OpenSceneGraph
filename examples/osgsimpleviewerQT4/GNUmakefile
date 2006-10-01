TOPDIR = ../..
include $(TOPDIR)/Make/makedefs

CXXFILES =\
	osgsimpleviewerQT4.cpp\

LIBS     += -losgGA -losgDB -losgUtil -losg -lQtOpenGL -lQtGui -lQtCore $(GL_LIBS) $(OTHER_LIBS)

INSTFILES = \
	$(CXXFILES)\
	GNUmakefile.inst=GNUmakefile

EXEC = osgsimpleviewerQT4

INC      += -I$(QT4_ROOT)/include
LDFLAGS  += -L$(QT4_ROOT)/lib

include $(TOPDIR)/Make/makerules 
