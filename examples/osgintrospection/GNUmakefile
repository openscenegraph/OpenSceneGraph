TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	osgintrospection.cpp\

LIBS     += -losgIntrospection -losgDB -losgUtil -losg $(GL_LIBS) $(OTHER_LIBS) 

INSTFILES = \
	$(CXXFILES)\
	GNUmakefile.inst=GNUmakefile

EXEC = osgintrospection

include $(TOPDIR)/Make/makerules 

