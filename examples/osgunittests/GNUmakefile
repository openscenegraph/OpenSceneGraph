TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	performance.cpp\
	UnitTestFramework.cpp\
	UnitTests_osg.cpp\
	osgunittests.cpp\

LIBS     += -losgProducer -lProducer -losgText -losgGA -losgDB -losgUtil -losg $(GL_LIBS) $(X_LIBS) $(OTHER_LIBS) 

INSTFILES = \
	$(CXXFILES)\
	GNUmakefile.inst=GNUmakefile

EXEC = osgunittests

INC +=  $(X_INC)

include $(TOPDIR)/Make/makerules 

