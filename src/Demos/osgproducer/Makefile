TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	osgproducer_viewer.cpp\
#	osgproducer_cameragroup.cpp\

LIBS     += -losgProducer -lProducer $(OSG_LIBS) $(GL_LIBS) $(X_LIBS) $(OTHER_LIBS) 

INSTFILES = \
	$(CXXFILES)\
	Makefile.inst=Makefile

EXEC = osgproducer

CXXFLAGS += $(OSG_INCLUDE_DIR)
LDFLAGS  += $(OSG_LIB_DIR)

include $(TOPDIR)/Make/makerules 

