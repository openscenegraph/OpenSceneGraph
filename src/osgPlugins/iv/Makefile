TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	main.cpp\
	nodecache.cpp\
	normals.cpp\
	osgvisitor.cpp\
	readerwriter.cpp\
	scanner.cpp\
	parser.cpp\

LIBS  += $(OSG_LIBS) $(OTHER_LIBS)

TARGET_BASENAME = iv

include $(TOPDIR)/Make/cygwin_plugin_def
PLUGIN = $(PLUGIN_PREFIX)$(TARGET_BASENAME).$(PLUGIN_EXT)

include $(TOPDIR)/Make/makerules 
