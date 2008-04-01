TOPDIR = ../.
include $(TOPDIR)/Make/makedefs

C++FILES =  \
	PThread.c++ \
        PThreadMutex.c++ \
        PThreadCondition.c++ \
        PThreadBarrier.c++ \
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

ifeq ($(OS),CYGWIN)
LIBS += -lpthread
DEF +=  -DOPENTHREADS_EXPORTS
endif

ifeq ($(OS),MINGW)
LIBS += -lpthread -lwinmm
DEF +=  -DOPENTHREADS_EXPORTS
endif

TARGET_BASENAME = OpenThreads
LIB = $(LIB_PREFIX)$(TARGET_BASENAME)

ifeq ($(COMPILE_USING_TWO_PARAM_sched_setaffinity),yes)
DEF += -DCOMPILE_USING_TWO_PARAM_sched_setaffinity
endif
include $(TOPDIR)/Make/makerules
