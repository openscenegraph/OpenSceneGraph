TOPDIR = ../..
include $(TOPDIR)/Make/makedefs
SHELL = /bin/sh

all :: rpm


rpm : osg.spec tardist 
	rpm -bb --clean osg.spec

osg.spec : version makespec
	./makespec 

tardist : 
	(cd $(TOPDIR)/..; \
		tar cf -  ${BINDIRS} | gzip > \
		/usr/src/redhat/SOURCES/osg-${OSGVERSION}.tar.gz)


clean : 
	rm -f osg.spec


version : version.o
	$(CXX) $(LDFLAGS) version.o -losg -o $@
