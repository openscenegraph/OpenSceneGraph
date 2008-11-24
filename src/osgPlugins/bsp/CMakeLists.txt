SET(TARGET_SRC
    ReaderWriterBSP.cpp
    BITSET.cpp
    Q3BSPReader.cpp
    Q3BSPLoad.cpp
    VBSPGeometry.cpp
    VBSPReader.cpp
)

SET(TARGET_H
    ReaderWriterBSP.h
    BITSET.h
    Q3BSPReader.h
    Q3BSPLoad.h
    VBSPGeometry.h
    VBSPReader.h
)

#### end var setup  ###
SETUP_PLUGIN(bsp)

