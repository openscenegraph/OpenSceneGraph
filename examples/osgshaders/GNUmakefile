TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	GL2Scene.cpp \
	osgshaders.cpp

LIBS     += -losgProducer -lProducer -losgText -losgGL2 -losgGA -losgDB -losgUtil -losg $(GL_LIBS) $(X_LIBS) $(OTHER_LIBS) 

INSTFILES = \
	$(CXXFILES)\
	GNUmakefile.inst=GNUmakefile

EXEC = osgshaders

INC += $(PRODUCER_INCLUDE_DIR) -I/usr/X11R6/include 
LDFLAGS  += $(PRODUCER_LIB_DIR)

include $(TOPDIR)/Make/makerules 

