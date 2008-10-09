SET(TARGET_SRC 
    UnitTestFramework.cpp 
    UnitTests_osg.cpp 
    osgunittests.cpp 
    performance.cpp
    MultiThreadRead.cpp
)

SET(TARGET_H 
    UnitTestFramework.h 
    performance.h
    MultiThreadRead.h
)

#### end var setup  ###

SETUP_COMMANDLINE_EXAMPLE(osgunittests)
