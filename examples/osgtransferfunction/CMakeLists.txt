SET(TARGET_SRC
    TransferFunctionWidget.cpp
    osgtransferfunction.cpp
)

SET(TARGET_ADDED_LIBRARIES osgVolume osgManipulator)

#### end var setup  ###
SETUP_EXAMPLE(osgtransferfunction)
