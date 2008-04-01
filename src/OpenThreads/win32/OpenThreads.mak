# Makefile - OpenThreads.dsp

ifndef CFG
CFG=OpenThreads - Win32 Release
endif
CC=gcc
CFLAGS=
CXX=g++
CXXFLAGS=$(CFLAGS)
RC=windres -O COFF
ifeq "$(CFG)"  "OpenThreads - Win32 Release"
CFLAGS+=-W -fexceptions -O2 -I../include -DWIN32 -DNDEBUG -D_WINDOWS -D_MBCS -D_USRDLL -DOPENTHREADS_EXPORTS
LD=g++
LDFLAGS=-shared -Wl,--out-implib,../bin/Win32/OpenThreadsWin32.dll.a -Wl,--export-all-symbols 
TARGET=../bin/Win32/OpenThreadsWin32.dll
LDFLAGS+=
LIBS+=-lkernel32 -luser32 -lgdi32 -lwinspool -lcomdlg32 -ladvapi32 -lshell32 -lole32 -loleaut32 -luuid -lodbc32 -lodbccp32
else
ifeq "$(CFG)"  "OpenThreads - Win32 Debug"
CFLAGS+=-W -fexceptions -g -O0 -I../include -DWIN32 -D_DEBUG -D_WINDOWS -D_MBCS -D_USRDLL -DOPENTHREADS_EXPORTS
LD=g++
LDFLAGS=-shared -Wl,--out-implib,../bin/Win32/OpenThreadsWin32d.dll.a -Wl,--export-all-symbols 
TARGET=../bin/Win32/OpenThreadsWin32d.dll
LDFLAGS+=
LIBS+=-lkernel32 -luser32 -lgdi32 -lwinspool -lcomdlg32 -ladvapi32 -lshell32 -lole32 -loleaut32 -luuid -lodbc32 -lodbccp32
endif
endif

ifndef TARGET
TARGET=OpenThreads.dll
endif

.PHONY: all
all: $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ -c $<

%.o: %.cc
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ -c $<

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ -c $<

%.o: %.cxx
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ -c $<

%.res: %.rc
	$(RC) $(CPPFLAGS) -o $@ -i $<

SOURCE_FILES= \
	WIN32Condition.cpp \
	Win32Mutex.cpp \
	Win32Thread.cpp \
	Win32ThreadBarrier.cpp

HEADER_FILES= \
	../include/OpenThreads/Barrier \
	../include/OpenThreads/Condition \
	../include/OpenThreads/Exports \
	../include/OpenThreads/Mutex \
	../include/OpenThreads/ScopedLock \
	../include/OpenThreads/Thread \
	Win32BarrierPrivateData.h \
	Win32Condition.h \
	Win32ConditionPrivateData.h \
	Win32MutexPrivateData.h \
	Win32ThreadPrivateData.h \
	HandleHolder.h

RESOURCE_FILES=

SRCS=$(SOURCE_FILES) $(HEADER_FILES) $(RESOURCE_FILES) 

OBJS=$(patsubst %.rc,%.res,$(patsubst %.cxx,%.o,$(patsubst %.cpp,%.o,$(patsubst %.cc,%.o,$(patsubst %.c,%.o,$(filter %.c %.cc %.cpp %.cxx %.rc,$(SRCS)))))))

$(TARGET): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

.PHONY: clean
clean:
	-rm -f $(OBJS) $(TARGET) OpenThreads.dep

.PHONY: depends
depends:
	-$(CXX) $(CXXFLAGS) $(CPPFLAGS) -MM $(filter %.c %.cc %.cpp %.cxx,$(SRCS)) > OpenThreads.dep

-include OpenThreads.dep

