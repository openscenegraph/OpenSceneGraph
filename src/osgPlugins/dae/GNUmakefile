TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
    domSourceReader.cpp \
    daeRMaterials.cpp  \
    daeRTransforms.cpp \
    daeRGeometry.cpp \
    daeRSceneObjects.cpp \
    daeReader.cpp  \
    daeWGeometry.cpp \
    daeWMaterials.cpp \
    daeWTransforms.cpp \
    daeWSceneObjects.cpp \
    daeWriter.cpp \
    ReaderWriterDAE.cpp

COLLADA_DAE_HOME ?= ${HOME}/3rdParty/collada/trunk
COLLADA_DAE_INCLUDES = -I$(COLLADA_DAE_HOME)/include -I$(COLLADA_DAE_HOME)/include/1.4

ifeq ($(COLLADA_DEBUG_LIBS),yes)
  COLLADA_DAE_LIBS = -L$(COLLADA_DAE_HOME)/lib-dbg
else
  COLLADA_DAE_LIBS = -L$(COLLADA_DAE_HOME)/lib
endif

COLLADA_DAE_LIBS +=  -lcollada_dae -lcollada_dom -lcollada_dae -lcollada_STLDatabase -lcollada_LIBXMLPlugin -lcollada_stdErrPlugin -lxml2 

INC += $(COLLADA_DAE_INCLUDES) -I$(THISDIR)  

LIBS  += $(OSG_LIBS) $(OTHER_LIBS) $(COLLADA_DAE_LIBS)  

TARGET_BASENAME = dae
include $(TOPDIR)/Make/cygwin_plugin_def
PLUGIN = $(PLUGIN_PREFIX)$(TARGET_BASENAME).$(PLUGIN_EXT)

include $(TOPDIR)/Make/makerules 
