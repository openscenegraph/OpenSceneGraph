TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	osgdem.cpp\

ifneq ("$(GDAL_INCLUDES)","")
INC += $(GDAL_INCLUDES)
endif

ifneq ("$(GDAL_LIB_DIR)","")
LDFLAGS += -L$(GDAL_LIB_DIR)
endif

LIBS     += -losgProducer -lProducer -losgTerrain -losgFX -losgGL2 -losgText -losgGA -losgDB -losgUtil -losg -lgdal $(GL_LIBS) $(X_LIBS) $(OTHER_LIBS) 

INSTFILES = \
	$(CXXFILES)\
	GNUmakefile.inst=GNUmakefile

EXEC = osgdem

INC +=  $(X_INC) 

include $(TOPDIR)/Make/makerules 

