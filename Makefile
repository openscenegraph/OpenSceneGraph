#!smake
SHELL=/bin/sh
MAKE_PREP = Make/makedefs Make/makerules

DIRS = src

VERSION = osg-0.8.42

all : $(MAKE_PREP)
	for f in $(DIRS) ; do cd $$f; $(MAKE) || exit 1; cd ..; done

docs :
	cd src; $(MAKE) docs;


Make/makedefs :
	@ cd Make;\
	 case `uname` in\
	 IRIX|IRIX64) \
	    ln -sf makedefs.irix.std makedefs ;;\
	 Linux) \
	    ln -sf makedefs.linux makedefs;;\
	 esac

Make/makerules :
	@ cd Make;\
	 case `uname` in\
	 IRIX|IRIX64) \
	    ln -sf makerules.irix makerules  ;; \
	 Linux) \
	    ln -sf makerules.linux makerules ;;\
	 esac

linux:
	cd Make;\
	ln -sf makedefs.linux makedefs;\
	ln -sf makerules.linux makerules
	$(MAKE)

freebsd:
	cd Make;\
	ln -sf makedefs.freebsd makedefs;\
	ln -sf makerules.freebsd makerules
	$(MAKE)


irix:
	cd Make;\
	ln -sf makedefs.irix.std makedefs ;\
	ln -sf makerules.irix makerules 
	$(MAKE)

irix.old:
	cd Make;\
	ln -sf makedefs.irix.nonstd makedefs ;\
	ln -sf makerules.irix makerules 
	$(MAKE)

macosx:
	cd Make;\
	ln -sf makedefs.macosx makedefs;\
	ln -sf makerules.macosx makerules
	$(MAKE)


help :
	@echo Usage : 
	@echo \	$(MAKE) 
	@echo \	$(MAKE) linux
	@echo \	$(MAKE) irix
	@echo \	$(MAKE) irix.old
	@echo \	$(MAKE) depend
	@echo \	$(MAKE) clean
	@echo \	$(MAKE) clobber
	@echo \	$(MAKE) doc
	@echo \	$(MAKE) release
	@echo \	$(MAKE) dev
	@echo \	$(MAKE) install
	@echo \	$(MAKE) instlinks
	@echo \	$(MAKE) instclean


clean : $(MAKE_PREP)
	for f in $(DIRS) ; do cd $$f; $(MAKE) clean; cd ..;  done
	rm -f `find lib -type f | grep -v .README`
	rm -f `ls bin/* | grep -v CVS`

clobber : $(MAKE_PREP) clean
	for f in $(DIRS) ; do cd $$f; $(MAKE) clobber; cd ..;  done
	rm -f $(MAKE_PREP)
	
depend : $(MAKE_PREP)
	for f in $(DIRS) ; do cd $$f; $(MAKE) depend; cd ..;  done

to_unix : 
	for f in $(DIRS) ; do cd $$f; to_unix Makefile Makefile; cd ..;  done
	for f in $(DIRS) ; do cd $$f; $(MAKE) to_unix; cd ..;  done
	cd include/osg; for f in * ; do to_unix $$f $$f; done
	cd include/osgUtil; for f in * ; do to_unix $$f $$f; done
	cd include/osgGLUT; for f in * ; do to_unix $$f $$f; done

beautify : 
	for f in $(DIRS) ; do cd $$f; $(MAKE) beautify; cd ..;  done
#	cd include/osg; for f in * ; do mv $$f $$f.bak; bcpp $$f.bak $$f; rm $$f.bak; done
#	cd include/osgUtil; for f in * ; do mv $$f $$f.bak; bcpp $$f.bak $$f; rm $$f.bak; done
#	cd include/osgGLUT; for f in * ; do mv $$f $$f.bak; bcpp $$f.bak $$f; rm $$f.bak; done

release: $(MAKE_PREP)
	$(MAKE) docs;
	$(MAKE) clobber;
	(cd ..; tar cvf - $(VERSION) | gzip > $(VERSION).tar.gz)

dev: $(MAKE_PREP)
	$(MAKE) docs;
	$(MAKE) clobber;
	(cd ..; tar cvf - $(VERSION) | gzip > osg-`date "+%Y%m%d"`.tar.gz)

install instlinks instclean :
	export OSGHOME=`pwd`; for f in $(DIRS)  ; do cd $$f; $(MAKE) $@; cd ..;  done

instcheck :
	diff -q include/osg/       /usr/include/osg/
	diff -q include/osgUtil/   /usr/include/osgUtil/
	diff -q include/osgDB/     /usr/include/osgDB/
	diff -q include/osgText/     /usr/include/osgText/
	diff -q include/osgGLUT/   /usr/include/osgGLUT/
	diff -q lib/libosg.so      /usr/lib/libosg.so
	diff -q lib/libosgUtil.so  /usr/lib/libosgUtil.so
	diff -q lib/libosgDB.so    /usr/lib/libosgDB.so
	diff -q lib/libosgGLUT.so  /usr/lib/libosgGLUT.so
	diff -q lib/libosgText.so  /usr/lib/libosgText.so
	diff -q lib/osgPlugins/    /usr/lib/osgPlugins/

stats :
	@echo stats : 
	cat include/osg/* src/osg/*.cpp | wc -l
	cat include/osgUtil/* src/osgUtil/*.cpp | wc -l
	cat include/osgDB/* src/osgDB/*.cpp | wc -l
	cat include/osgGLUT/* src/osgGLUT/*.cpp | wc -l
	cat include/osgWX/* src/osgWX/*.cpp | wc -l
	cat include/*/* src/*/*.cpp | wc -l
	cat src/Demos/*/*.cpp | wc -l
	cat src/osgPlugins/*/*.cpp src/osgPlugins/*/*.h | wc -l
	cat include/*/* src/*/*.cpp src/Demos/*/*.cpp src/osgPlugins/*/*.cpp src/osgPlugins/*/*.h | wc -l
