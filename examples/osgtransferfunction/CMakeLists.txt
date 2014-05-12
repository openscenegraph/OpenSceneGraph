SET(TARGET_SRC
    TransferFunctionWidget.cpp
    osgtransferfunction.cpp
)

SET(TARGET_ADDED_LIBRARIES osgVolume osgUI osgManipulator)

#### end var setup  ###
SETUP_EXAMPLE(osgtransferfunction)
