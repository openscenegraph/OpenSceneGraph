TOPDIR = ../.
include $(TOPDIR)/Make/makedefs

C++FILES =  \
	SprocThread.c++ \
        SprocMutex.c++ \
        SprocBarrier.c++ \
        SprocCondition.c++ \
        SharedArena.c++ \
        $(NULL)

INC += -I$(TOPDIR)/include -I.

TARGET_BASENAME = OpenThreads
LIB = libOpenThreads

include $(TOPDIR)/Make/makerules
