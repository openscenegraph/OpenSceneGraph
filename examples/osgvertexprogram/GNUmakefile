TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	osgvertexprogram.cpp\

LIBS     += -losgProducer -lProducer -losgGA -losgDB -losgUtil -losg $(GL_LIBS) $(X_LIBS) $(OTHER_LIBS) 

INSTFILES = \
	$(CXXFILES)\
	GNUmakefile.inst=GNUmakefile

EXEC = osgvertexprogram

INC += $(PRODUCER_INCLUDE_DIR)
LDFLAGS  += $(PRODUCER_LIB_DIR)

include $(TOPDIR)/Make/makerules 
