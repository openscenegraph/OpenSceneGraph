TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	broadcaster.cpp\
	osgcluster.cpp\
	receiver.cpp\

HEADERFILES =\
	broadcaster.h\
	receiver.h\


LIBS     += -losgProducer -lProducer -losgText -losgGA -losgDB -losgUtil -losg $(GL_LIBS) $(X_LIBS) $(SOCKET_LIBS) $(OTHER_LIBS) 

INSTFILES = \
	$(CXXFILES)\
	$(HEADERFILES)\
	GNUmakefile.inst=GNUmakefile

EXEC = osgcluster


INC +=  $(X_INC)

include $(TOPDIR)/Make/makerules 
