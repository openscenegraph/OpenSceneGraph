#!smake
SHELL=/bin/sh
MAKE_PREP = Make/makedefs Make/makerules

DIRS = src


all : $(MAKE_PREP)
	for f in $(DIRS) ; do cd $$f; make; cd ..; done

docs :
	cd src; make docs;


Make/makedefs :
	@ cd Make;\
	 case `uname` in\
	 IRIX|IRIX64) \
	    ln -sf makedefs.irix.nonstd makedefs ;;\
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
	make

irix:
	cd Make;\
	ln -sf makedefs.irix.nonstd makedefs ;\
	ln -sf makerules.irix makerules 
	make

irix.std:
	cd Make;\
	ln -sf makedefs.irix.std makedefs ;\
	ln -sf makerules.irix makerules 
	make

help :
	@echo Usage : 
	@echo \	make 
	@echo \	make linux
	@echo \	make irix.std
	@echo \	make irix.nonstd
	@echo \	make depend
	@echo \	make clean
	@echo \	make clobber
	@echo \	make doc
	@echo \	make snapshot
	@echo \	make install
	@echo \	make instlinks
	@echo \	make instclean


clean : $(MAKE_PREP)
	for f in $(DIRS) ; do cd $$f; make clean; cd ..;  done
	find lib -type f -exec rm {} \;
	rm -f bin/*

clobber : $(MAKE_PREP) clean
	for f in $(DIRS) ; do cd $$f; make clobber; cd ..;  done
	rm -f $(MAKE_PREP)
	
depend : $(MAKE_PREP)
	for f in $(DIRS) ; do cd $$f; make depend; cd ..;  done

to_unix : 
	for f in $(DIRS) ; do cd $$f; to_unix Makefile Makefile; cd ..;  done
	for f in $(DIRS) ; do cd $$f; make to_unix; cd ..;  done
	cd include/OSG; for f in *.h ; do to_unix $$f $$f;  done

snapshot: 
	make doc;
	make clobber;
	(cd ..; tar cvf - OpenSceneGraph-0.8 | gzip > osg_src-`date "+%Y%m%d"`.tar.gz)


install :
	for f in $(DIRS)  ; do cd $$f; make install; cd ..;  done

instlinks :
	for f in $(DIRS)  ; do cd $$f; make instlinks; cd ..;  done

instclean :
	for f in $(DIRS)  ; do cd $$f; make instclean; cd ..;  done

