SET(TARGET_SRC
    Widget.cpp
    TransferFunctionWidget.cpp
    osgtransferfunction.cpp
)

SET(TARGET_ADDED_LIBRARIES osgVolume osgManipulator)

#### end var setup  ###
SETUP_EXAMPLE(osgtransferfunction)
