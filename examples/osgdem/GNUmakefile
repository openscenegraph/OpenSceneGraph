TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	osgdem.cpp\

INC += $(GDAL_INCLUDES)

LIBS     += -losgProducer -lProducer -losgTerrain -losgFX -losgGL2 -losgText -losgGA -losgDB -losgUtil -losg $(GDAL_LIBS) $(GL_LIBS) $(X_LIBS) $(OTHER_LIBS) 

ifeq ($(OS),MINGW)
LIBS	+= -ljpeg -lpng -lgeotiff -ltiff -lz -L/usr/mingw/lib/malloc.o
endif

INSTFILES = \
	$(CXXFILES)\
	GNUmakefile.inst=GNUmakefile

EXEC = osgdem

INC +=  $(X_INC) 

include $(TOPDIR)/Make/makerules 

