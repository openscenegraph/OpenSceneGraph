TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	main.cpp\

LINKARGS += -L/usr/local/lib
LIBS     += -losgText $(OSG_LIBS) $(FREETYPE_LIB) $(GLUT_LIB) $(GL_LIBS) $(X_LIBS) $(OTHER_LIBS)  

INSTFILES = \
	$(CXXFILES)\
	Makefile.inst=Makefile

EXEC = osgtext

include $(TOPDIR)/Make/makerules 
