TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	ProducerEventCallback.cpp\
	ProducerEventAdapter.cpp\
	osgproducer.cpp\

LIBS     += -lProducer $(OSG_LIBS) $(GL_LIBS) $(X_LIBS) $(OTHER_LIBS) 

INSTFILES = \
	$(CXXFILES)\
	Makefile.inst=Makefile

EXEC = osgproducer

include $(TOPDIR)/Make/makerules 

