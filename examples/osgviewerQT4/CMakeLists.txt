#configure what we need from Qt4
SET(QT_USE_QTOPENGL yes)
INCLUDE(UseQt4)

SET(TARGET_SRC osgsimpleviewerQT4.cpp )

SET(TARGET_EXTERNAL_LIBRARIES ${QT_LIBRARIES} )

INCLUDE_DIRECTORIES(${QT_INCLUDE_DIR} )

#### end var setup  ###
SETUP_EXAMPLE(osgsimpleviewerQT4)
