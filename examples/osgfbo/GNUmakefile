TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	FBOExtensions.cpp\
	FramebufferObject.cpp\
	Renderbuffer.cpp\
	FramebufferAttachment.cpp\
	osgfbo.cpp\

LIBS     += -losgProducer -lProducer -losgText -losgGA -losgDB -losgUtil -losg $(GL_LIBS) $(X_LIBS) $(OTHER_LIBS) 

INSTFILES = \
	$(CXXFILES)\
	GNUmakefile.inst=GNUmakefile

EXEC = osgfbo

INC +=  $(X_INC) 

include $(TOPDIR)/Make/makerules 

