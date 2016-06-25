SET(TARGET_SRC
    Block.cpp
    Clip.cpp
    Converter.cpp
    Object.cpp
    old_lw.cpp
    Polygon.cpp
    ReaderWriterLWO.cpp
    Surface.cpp
    Tessellator.cpp
    Unit.cpp
    VertexMap.cpp
)

SET(TARGET_H
    Block.h
    Clip.h
    Converter.h
    iffparser.h
    Layer.h
    lwo2chunks.h
    lwo2parser.h
    lwo2read.h
    lwo2types.h
    Object.h
    old_lw.h
    Polygon.h
    Surface.h
    Tessellator.h
    Unit.h
    VertexMap.h
)

SET(TARGET_ADDED_LIBRARIES osgFX)

#### end var setup  ###
SETUP_PLUGIN(lwo)
