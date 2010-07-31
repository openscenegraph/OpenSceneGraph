INCLUDE(OsgMacroUtils)

SET(TARGET_SRC
    POVWriterNodeVisitor.cpp
    ReaderWriterPOV.cpp
)
SET(TARGET_HDRS
    POVWriterNodeVisitor.h
    ReaderWriterPOV.h
)

SETUP_PLUGIN(pov)
