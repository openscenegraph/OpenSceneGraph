TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	osgproducer_viewer.cpp\
#	osgproducer_cameragroup.cpp\

LIBS     += -losgProducer $(OSG_LIBS) $(GL_LIBS) $(X_LIBS) $(OTHER_LIBS) 

INSTFILES = \
	$(CXXFILES)\
	Makefile.inst=Makefile

EXEC = osgproducer

include $(TOPDIR)/Make/makerules 

