TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	osgtext.cpp\

LIBS     += -losgText -losgProducer -lProducer $(OSG_LIBS) -L/usr/local/lib $(FREETYPE_LIB) $(GLUT_LIB) $(GL_LIBS) $(X_LIBS) $(OTHER_LIBS)  

INSTFILES = \
	$(CXXFILES)\
	Makefile.inst=Makefile

EXEC = osgtext

include $(TOPDIR)/Make/makerules 
