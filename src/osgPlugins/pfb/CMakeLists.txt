INCLUDE_DIRECTORIES( ${PERFORMER_INCLUDE_DIR} )

SET(TARGET_SRC 
    ReaderWriterPFB.cpp
    ConvertFromPerformer.cpp
)

SET(TARGET_H 
    ConvertFromPerformer.h
)

SET(TARGET_EXTERNAL_LIBRARIES 
    pf3ds pfdem pfdted pfdwb pfdxf pfevt
    pfflt pfgds pfgfo pfim pflsa pflsb
    pfnff pfobj pfpegg pfpfb pfphd pfpts
    pfrot pfscale pfsgf pfsgo pfsphere pfsv
    pftri pfunc pfvct pfdu pfutil pfdb
)

SET(TARGET_LIBRARIES_VARS PERFORMER_LIBRARY )

#### end var setup  ###
SETUP_PLUGIN(pfb)
