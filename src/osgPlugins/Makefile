TOPDIR = ../..
include $(TOPDIR)/Make/makedefs

DIRS = osg rgb lib3ds flt obj lwo dw bmp pic tga osgtgz tgz zip  

# The following are giving me problems on SUN
#DIRS += txp

# comment in if you have Performer installed.
# DIRS += pfb

# comment in if you want Open DX support, currently not on by default
# since we havn't yet checked compilation on all platforms yet.
DIRS += dx

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



include $(TOPDIR)/Make/makedirrules
