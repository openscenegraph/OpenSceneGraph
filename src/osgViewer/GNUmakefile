TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 


CXXFILES = \
	CompositeViewer.cpp\
	Scene.cpp\
	SimpleViewer.cpp\
	StatsHandler.cpp\
	HelpHandler.cpp\
	Version.cpp\
	View.cpp\
	Viewer.cpp\


ifeq ($(OS),Darwin)
  ifeq ($(USE_AGL),yes)
     CXXFILES += GraphicsWindowCarbon.cpp
     LIBS     += -framework AGL
  else 
    ifeq ($(USE_CGL),yes)
       CXXFILES += GraphicsWindowCarbon.cpp
    else
       CXXFILES += GraphicsWindowX11.cpp
       LIBS     += $(X_LIBS)
    endif
  endif
else
  ifeq ($(OS),MINGW)
     CXXFILES += GraphicsWindowWin32.cpp
     LIBS += -lgdi32
  else
    ifeq ($(OS),CYGWIN)
       CXXFILES += GraphicsWindowWin32.cpp
       LIBS += -lgdi32
    else
       CXXFILES += GraphicsWindowX11.cpp
    endif
  endif
endif

DEF     += -DOSGVIEWER_LIBRARY

LIBS  += -losgGA -losgText -losgDB -losgUtil -losg $(GL_LIBS) $(OTHER_LIBS)

TARGET_BASENAME = osgViewer
LIB = $(LIB_PREFIX)$(TARGET_BASENAME).$(LIB_EXT)

include $(TOPDIR)/Make/makerules 
