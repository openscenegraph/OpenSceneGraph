TOPDIR = .
include $(TOPDIR)/Make/makedefs

DIRS = src

include $(TOPDIR)/Make/makedirrules
include $(TOPDIR)/Make/instrules
include $(TOPDIR)/Make/distrules
