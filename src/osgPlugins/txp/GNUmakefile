TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
        ReaderWriterTXP.cpp\
        TXPArchive.cpp\
        TXPIO.cpp\
        TXPNode.cpp\
        TXPPageManager.cpp\
        TXPParser.cpp\
        TXPSeamLOD.cpp\
        TXPPagedLOD.cpp\
        TileMapper.cpp\
        trpage_basic.cpp\
        trpage_compat.cpp\
        trpage_geom.cpp\
        trpage_header.cpp\
        trpage_label.cpp\
        trpage_light.cpp\
        trpage_main.cpp\
        trpage_managers.cpp\
        trpage_material.cpp\
        trpage_model.cpp\
        trpage_nodes.cpp\
        trpage_parse.cpp\
        trpage_pparse.cpp\
        trpage_print.cpp\
        trpage_print_parse.cpp\
        trpage_range.cpp\
        trpage_rarchive.cpp\
        trpage_readbuf.cpp\
        trpage_scene.cpp\
        trpage_swap.cpp\
        trpage_tile.cpp\
        trpage_util.cpp\
        trpage_warchive.cpp\
        trpage_writebuf.cpp\


INC += -I$(THISDIR)

LIBS  += -losgSim -losgText $(OSG_LIBS) $(OTHER_LIBS)

TARGET_BASENAME = txp
include $(TOPDIR)/Make/cygwin_plugin_def
PLUGIN = $(PLUGIN_PREFIX)$(TARGET_BASENAME).$(PLUGIN_EXT)

#### LIB NOT NEEDED ANYMORE and causes a race condition when doing a parallel make.
####LIB_TARGET_BASENAME=osgTXP
####LIB = $(LIB_PREFIX)$(LIB_TARGET_BASENAME).$(LIB_EXT)

include $(TOPDIR)/Make/makerules 
