SET(TARGET_SRC
    ReaderWriterTXP.cpp
    TileMapper.cpp
    trpage_basic.cpp
    trpage_compat.cpp
    trpage_geom.cpp
    trpage_header.cpp
    trpage_label.cpp
    trpage_light.cpp
    trpage_main.cpp
    trpage_managers.cpp
    trpage_material.cpp
    trpage_model.cpp
    trpage_nodes.cpp
    trpage_parse.cpp
    trpage_pparse.cpp
    trpage_print.cpp
    trpage_print_parse.cpp
    trpage_range.cpp
    trpage_rarchive.cpp
    trpage_readbuf.cpp
    trpage_scene.cpp
    trpage_swap.cpp
    trpage_tile.cpp
    trpage_util.cpp
    trpage_warchive.cpp
    trpage_writebuf.cpp
    TXPArchive.cpp
    TXPIO.cpp
    TXPNode.cpp
    TXPPagedLOD.cpp
    TXPPageManager.cpp
    TXPParser.cpp
    TXPSeamLOD.cpp
)
SET(TARGET_H
    ReaderWriterTXP.h
    TileMapper.h
    trdll.h
    trpage_compat.h
    trpage_geom.h
    trpage_ident.h
    trpage_io.h
    trpage_managers.h
    trpage_print.h
    trpage_read.h
    trpage_scene.h
    trpage_swap.h
    trpage_sys.h
    trpage_util.h
    trpage_write.h
    TXPArchive.h
    TXPNode.h
    TXPPagedLOD.h
    TXPPageManager.h
    TXPParser.h
    TXPSeamLOD.h
)

INCLUDE_DIRECTORIES(${OpenSceneGraph_SOURCE_DIR}/src/osgPlugins/txp)

SET(TARGET_ADDED_LIBRARIES osgSim osgText)

#### end var setup  ###
SETUP_PLUGIN(txp)

