SHELL = /bin/sh

BINDIRS = \
	OpenSceneGraph-${OSGVERSION}/bin/\
	OpenSceneGraph-${OSGVERSION}/include/\
	OpenSceneGraph-${OSGVERSION}/lib/\

all :: rpm


rpm : osg.spec tardist 
	rpm -bb --clean osg.spec


osg.spec : makespec
	makespec 

tardist : 
	@[ -n "${OSGHOME}" ] || (echo Please define OSGHOME; exit 3)
	@[ -n "${OSGVERSION}" ] || (echo Please define OSGVERSION; exit 4)
	@[ -n "${OSGRELEASE}" ] || (echo Please define OSGRELEASE; exit 5)
	@[ "`whoami`" = "root" ] || (echo Must be ROOT; exit 6)
	(cd ${OSGHOME}/..; \
		tar cf -  ${BINDIRS} | gzip > \
		/usr/src/redhat/SOURCES/osg-${OSGVERSION}-${OSGRELEASE}.tar.gz)


clean : 
	rm -f osg.spec
