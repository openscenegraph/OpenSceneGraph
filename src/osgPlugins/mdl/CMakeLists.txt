SET(TARGET_SRC
    ReaderWriterMDL.cpp
    BodyPart.cpp
    Mesh.cpp
    Model.cpp
    MDLReader.cpp
    MDLRoot.cpp
    VTXReader.cpp
    VVDReader.cpp
)

SET(TARGET_H
    ReaderWriterMDL.h
    BodyPart.h
    Mesh.h
    Model.h
    MDLLimits.h
    MDLReader.h
    MDLRoot.h
    VTXReader.h
    VVDReader.h
)

#### end var setup  ###
SETUP_PLUGIN(mdl)

