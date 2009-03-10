SET(TARGET_SRC ReaderWriterImageIO.cpp )

SET(TARGET_ADDED_LIBRARIES)
SET(TARGET_LIBRARIES_VARS IMAGEIO_LIBRARY )
SET(TARGET_EXTERNAL_LIBRARIES "/System/Library/Frameworks/Accelerate.framework" )

#### end var setup  ###
SETUP_PLUGIN(imageio)

