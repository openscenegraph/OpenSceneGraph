TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
        SlideShowConstructor.cpp\
        SlideEventHandler.cpp\
        DefaultPresentation.cpp\
        ReaderWriterXML.cpp\
	slideshow3D.cpp\

CXXFLAGS += -I/usr/include/libxml2
LIBS     += -lxml2 -losgProducer -lProducer -losgText -losgGA -losgDB -losgUtil -losg $(GL_LIBS) $(X_LIBS) $(OTHER_LIBS) 

INSTFILES = \
	$(CXXFILES)\
	GNUmakefile.inst=GNUmakefile

EXEC = slideshow3D

INC += $(PRODUCER_INCLUDE_DIR) -I/usr/X11R6/include 
LDFLAGS  += $(PRODUCER_LIB_DIR)

include $(TOPDIR)/Make/makerules 

