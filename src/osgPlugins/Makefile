#!smake
SHELL=/bin/sh

#DIRS = osg rgb lib3ds flt pfb obj lwo dw osgtgz tgz zip pic png jpeg tga tiff gif
DIRS = osg rgb lib3ds flt obj lwo dw osgtgz tgz zip pic png jpeg tga tiff gif

all :
	for f in $(DIRS) ; do cd $$f; $(MAKE) || exit 1; cd ..; done

clean :
	for f in $(DIRS) ; do cd $$f; $(MAKE) clean; cd ..;  done

clobber : 
	for f in $(DIRS) ; do cd $$f; $(MAKE) clobber; cd ..;  done

depend : 
	for f in $(DIRS) ; do cd $$f; $(MAKE) depend; cd ..;  done

to_unix : 
	for f in $(DIRS) ; do cd $$f; to_unix Makefile Makefile; cd ..;  done
	for f in $(DIRS) ; do cd $$f; $(MAKE) to_unix; cd ..;  done

beautify : 
	for f in $(DIRS) ; do cd $$f; $(MAKE) beautify; cd ..;  done

install :
	for f in $(DIRS)  ; do cd $$f; $(MAKE) install; cd ..;  done

instlinks :
	for f in $(DIRS)  ; do cd $$f; $(MAKE) instlinks; cd ..;  done

instclean :
	for f in $(DIRS)  ; do cd $$f; $(MAKE) instclean; cd ..;  done
