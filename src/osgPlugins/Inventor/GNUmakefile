TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
    ReaderWriterIV.cpp \
    ConvertFromInventor.cpp \
    PendulumCallback.cpp \
    ShuttleCallback.cpp \
    GroupSoLOD.cpp
    
ifeq ($(USE_COIN),1)
    INVENTOR_LIB = -lCoin
else
    INVENTOR_LIB = -lInventor
endif

INC += -I$(THISDIR)
LIBS  += $(OSG_LIBS) $(OTHER_LIBS) $(INVENTOR_LIB)
TARGET_BASENAME = iv

include $(TOPDIR)/Make/cygwin_plugin_def
PLUGIN = $(PLUGIN_PREFIX)$(TARGET_BASENAME).$(PLUGIN_EXT)

include $(TOPDIR)/Make/makerules 
