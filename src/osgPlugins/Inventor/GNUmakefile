TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
    ReaderWriterIV.cpp \
    ConvertFromInventor.cpp \
    PendulumCallback.cpp \
    ShuttleCallback.cpp \
    GroupSoLOD.cpp
    
ifeq ($(COIN_INSTALLED),yes)
    INVENTOR_LIB = -lCoin
else
    INVENTOR_LIB = -lInventor
endif

INC += -I$(THISDIR) $(INVENTOR_INCLUDE)
LIBS  += $(OSG_LIBS) $(OTHER_LIBS) $(INVENTOR_LIB_DIR) $(INVENTOR_LIB)
TARGET_BASENAME = iv

include $(TOPDIR)/Make/cygwin_plugin_def
PLUGIN = $(PLUGIN_PREFIX)$(TARGET_BASENAME).$(PLUGIN_EXT)

include $(TOPDIR)/Make/makerules 
