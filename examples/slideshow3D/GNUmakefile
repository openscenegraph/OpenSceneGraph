TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
        SlideShowConstructor.cpp\
        SlideEventHandler.cpp\
        DefaultPresentation.cpp\
        ReaderWriterXML.cpp\
	slideshow3D.cpp\

LIBS     += -lxml2 -losgProducer -lProducer -losgText -losgGA -losgDB -losgUtil -losg $(GL_LIBS) $(X_LIBS) $(OTHER_LIBS) 

INSTFILES = \
	$(CXXFILES)\
	GNUmakefile.inst=GNUmakefile

EXEC = slideshow3D

INC +=  $(X_INC) $(LIBXML2_INCLUDE)

include $(TOPDIR)/Make/makerules 

