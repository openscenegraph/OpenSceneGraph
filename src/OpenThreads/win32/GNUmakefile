TOPDIR = ../.
include $(TOPDIR)/Make/makedefs

CXXFILES =  \
	WIN32Condition.cpp \
	Win32Mutex.cpp \
	Win32Thread.cpp \
	Win32ThreadBarrier.cpp \
        $(NULL)

INC += -I$(TOPDIR)/include -I.

ifeq ($(OS),Linux) 
DEF += -fPIC -DLinux -DGL_GLEXT_PROTOTYPES
LIBS += -lpthread
endif 

ifeq ($(OS),SunOS)
LIBS +=  -lpthread -lposix4
endif

ifeq ($(OS),IRIX)
LIBS += -lpthread
endif

ifeq ($(OS),Darwin)
LIBS += -lpthread
endif

ifeq ($(OS),MINGW)
LIBS +=
DEF +=  -DOPENTHREADS_EXPORTS
endif

TARGET_BASENAME = OpenThreads
LIB = $(LIB_PREFIX)$(TARGET_BASENAME)

include $(TOPDIR)/Make/makerules
