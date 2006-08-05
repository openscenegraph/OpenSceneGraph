TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	osgcegui.cpp\

LIBS     += -lCEGUIBase -lCEGUIOpenGLRenderer  -losgProducer -lProducer -losgText -losgGA -losgDB -losgUtil -losg $(GL_LIBS) $(X_LIBS) $(OTHER_LIBS) 

INSTFILES = \
	$(CXXFILES)\
	GNUmakefile.inst=GNUmakefile

INC += -I/usr/local/include/CEGUI

EXEC = osgcegui

INC +=  $(X_INC) 

include $(TOPDIR)/Make/makerules 

