#!smake
SHELL=/bin/sh


DIRS = osg osgUtil osgGLUT Demos osgPlugins

all :
	for f in $(DIRS) ; do cd $$f; make ; cd ..; done

clean :
	for f in $(DIRS) ; do cd $$f; make clean; cd ..;  done

clobber : 
	for f in $(DIRS) ; do cd $$f; make clobber; cd ..;  done

depend : 
	for f in $(DIRS) ; do cd $$f; make depend; cd ..;  done

docs : 
	(cd osg; make docs;) 
	(cd osgUtil; make docs; )
	(cd osgGLUT; make docs; )

to_unix : 
	for f in $(DIRS) ; do cd $$f; to_unix Makefile Makefile; cd ..;  done
	for f in $(DIRS) ; do cd $$f; make to_unix; cd ..;  done


install :
	for f in $(DIRS)  ; do cd $$f; make install; cd ..;  done

instlinks :
	for f in $(DIRS)  ; do cd $$f; make instlinks; cd ..;  done

instclean :
	for f in $(DIRS)  ; do cd $$f; make instclean; cd ..;  done
