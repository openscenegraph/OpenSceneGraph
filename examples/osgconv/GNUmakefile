TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	OrientationConverter.cpp\
	osgconv.cpp\

HEADERFILES = \
	OrientationConverter.h

LIBS     += -losg -losgUtil -losgDB $(GL_LIBS) $(OTHER_LIBS) 

INSTFILES = \
	$(CXXFILES)\
	$(HEADERFILES)\
	Makefile.inst=Makefile

EXEC = osgconv

include $(TOPDIR)/Make/makerules 
