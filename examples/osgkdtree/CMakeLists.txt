SET(TARGET_SRC 
    fixeddivision.cpp
    variabledivision.cpp
    osgkdtree.cpp
)

SET(TARGET_H 
    fixeddivision.h
    variabledivision.h
)


SET(TARGET_ADDED_LIBRARIES osgSim )

#### end var setup  ###
SETUP_EXAMPLE(osgkdtree)
