TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	osgstereoimage.cpp\

LIBS     += -losgProducer -lProducer $(OSG_LIBS) -L/usr/local/lib $(FREETYPE_LIB) $(GLUT_LIB) $(GL_LIBS) $(X_LIBS) $(OTHER_LIBS)  

INSTFILES = \
	$(CXXFILES)\
	Makefile.inst=Makefile

EXEC = osgstereoimage

include $(TOPDIR)/Make/makerules 
