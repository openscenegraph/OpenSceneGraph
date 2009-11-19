SET(TARGET_SRC osgintrospection.cpp )
SET(TARGET_ADDED_LIBRARIES osgIntrospection )

ADD_DEFINITIONS(-DOSG_PLUGIN_EXTENSION=${CMAKE_SHARED_MODULE_SUFFIX})

#### end var setup  ###
SETUP_EXAMPLE(osgintrospection)
