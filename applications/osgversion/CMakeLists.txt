SET(TARGET_SRC osgversion.cpp )

SET(TARGET_COMMON_LIBRARIES 
    OpenThreads
    osg 
)

SETUP_COMMANDLINE_APPLICATION(osgversion)
