TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
    ReaderWriterTXP.cpp\
    TerrapageNode.cpp\
    IO_TerrapageNode.cpp\
    TrPageArchive.cpp\
    TrPageParser.cpp\
    trpage_basic.cpp\
    trpage_geom.cpp\
    trpage_header.cpp\
    trpage_tile.cpp\
    trpage_readbuf.cpp\
    trpage_rarchive.cpp\
    trpage_writebuf.cpp\
    trpage_warchive.cpp\
    trpage_parse.cpp\
    trpage_nodes.cpp\
    trpage_model.cpp\
    trpage_material.cpp\
    trpage_swap.cpp\
    trpage_range.cpp\
    trpage_scene.cpp\
    trpage_compat.cpp\
    trpage_light.cpp\
    trpage_pparse.cpp\
    trpage_print.cpp\
    trpage_managers.cpp\
    trPagePageManager.cpp\
    trpage_print_parse.cpp\
    trpage_util.cpp\



INC += -I$(THISDIR) 

LIBS  += $(OSG_LIBS) $(OTHER_LIBS)

PLUGIN_TARGET_BASENAME = txp
include $(TOPDIR)/Make/cygwin_plugin_def
PLUGIN = $(PLUGIN_PREFIX)$(PLUGIN_TARGET_BASENAME).$(PLUGIN_EXT)

#### LIB NOT NEEDED ANYMORE and causes a race condition when doing a parallel make.
####LIB_TARGET_BASENAME=osgTXP
####LIB = $(LIB_PREFIX)$(LIB_TARGET_BASENAME).$(LIB_EXT)

include $(TOPDIR)/Make/makerules 
