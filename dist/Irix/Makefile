SHELL = /bin/sh


all : irixdist tardist


irixdist :
	cd ${OSGHOME}; [ -d irixdist ] || mkdir irixdist
	cd ${OSGHOME}; \
	/usr/sbin/gendist -sbase ${OSGHOME} -idb dist/Irix/osg.idb -spec dist/Irix/osg.spec -dist irixdist -all


tardist : irixdist
	cd ${OSGHOME}/irixdist;\
	tar cvf osg.tardist osg  osg.idb  osg.sw

