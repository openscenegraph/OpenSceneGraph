TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	osgarchive.cpp\

LIBS     += -losgProducer -lProducer -losgDB -losgUtil -losg $(GDAL_LIBS) $(GL_LIBS) $(X_LIBS) $(OTHER_LIBS) 

INSTFILES = \
	$(CXXFILES)\
	GNUmakefile.inst=GNUmakefile

EXEC = osgarchive

INC +=  $(X_INC) 

include $(TOPDIR)/Make/makerules 

