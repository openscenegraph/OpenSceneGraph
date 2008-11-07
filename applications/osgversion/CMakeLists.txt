SET(TARGET_SRC osgversion.cpp )

SET(TARGET_COMMON_LIBRARIES 
    OpenThreads
    osg 
    osgDB
)

SETUP_COMMANDLINE_APPLICATION(osgversion)
