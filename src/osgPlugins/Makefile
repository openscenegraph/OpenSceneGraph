#!gmake
SHELL=/bin/sh

DIRS = osg rgb lib3ds flt obj lwo txp dw bmp pic tga osgtgz tgz zip  

# comment in if you have Performer installed.
# DIRS += pfb

# comment in if you want Open DX support, currently not in by default
# since we havn't yet checked compilation on all platforms yet.
# DIRS += dx

# comment in if you have Quicktime installed, i.e under Mac OS.
# if in this case then its likely you'll want to comment out the below
# png, jpeg, gif and tiff plugins.
# DIRS += quicktime

# comment in if you have libpng installed.
DIRS += png

# comment in if you have libjpeg installed.
DIRS += jpeg

# comment in if you have libungif installed.
DIRS += gif

# comment in if you have libtiff installed.
DIRS += tiff

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
