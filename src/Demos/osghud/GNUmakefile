TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	osghud.cpp\

#LINKARGS += -L/usr/local/lib
LIBS     += -losgText $(OSG_LIBS) -L/usr/local/lib $(FREETYPE_LIB) $(GLUT_LIB) $(GL_LIBS) $(X_LIBS) $(OTHER_LIBS)  

INSTFILES = \
	$(CXXFILES)\
	Makefile.inst=Makefile

EXEC = osghud

include $(TOPDIR)/Make/makerules 
