#!smake
SHELL=/bin/sh

DIRS = sgv osgconv osgcube osgreflect osgtexture osgimpostor osgviews hangglide osgcluster

# comment out if you don't have the freetype and GLU1.3 library installed.
DIRS += osgtext 

# comment out if you don't have the wxWindows installed.
# DIRS += wxsgv

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
	for f in $(DIRS) ; do cd $$f; $(MAKE)to_unix; cd ..;  done

beautify : 
	for f in $(DIRS) ; do cd $$f; $(MAKE)beautify; cd ..;  done

install :
	for f in $(DIRS)  ; do cd $$f; $(MAKE) install; cd ..;  done

instlinks :
	for f in $(DIRS)  ; do cd $$f; $(MAKE) instlinks; cd ..;  done

instclean :
	for f in $(DIRS)  ; do cd $$f; $(MAKE) instclean; cd ..;  done
