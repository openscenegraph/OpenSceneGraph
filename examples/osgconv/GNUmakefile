TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
        GeoSet.cpp\
        IO_GeoSet.cpp\
	OrientationConverter.cpp\
	osgconv.cpp\

HEADERFILES = \
	OrientationConverter.h

LIBS     += -losgProducer -lProducer -losg -losgUtil -losgGA -losgDB $(GL_LIBS) $(OTHER_LIBS)  

INSTFILES = \
	$(CXXFILES)\
	$(HEADERFILES)\
	GNUmakefile.inst=GNUmakefile

EXEC = osgconv

include $(TOPDIR)/Make/makerules 
