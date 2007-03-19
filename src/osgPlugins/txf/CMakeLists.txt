SET(TARGET_NAME txf )
SET(TARGET_SRC 
    ReaderWriterTXF.cpp
    TXFFont.cpp
)
SET(TARGET_H
    TXFFont.h
)
SET(TARGET_ADDED_LIBRARIES osgText )

#### end var setup  ###
SETUP_PLUGIN()
