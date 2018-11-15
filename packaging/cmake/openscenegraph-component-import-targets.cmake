# This is a utility file for importing the target (and its dependencies) of
# @LIB_NAME@, a component library of OpenSceneGraph. This is designed to be used
# by OpenSceneGraphConfig.cmake and should not be invoked directly.

# First we import all dependent targets (as well as their dependencies, recursively)
foreach(dependency @COMPONENT_CMAKE_DEPENDENCIES@)
    set(osg_component_dependency_target_file ${CMAKE_CURRENT_LIST_DIR}/openscenegraph-${dependency}-import-targets.cmake)
    if(NOT EXISTS ${osg_component_dependency_target_file})
        # Note: This should never happen, because if this library was installed
        # and its target was exported, then its dependencies should have been
        # installed with their targets alongside it. If we ever land here, then
        # either there is a bug in the build system, or the user has broken
        # their installation of OpenSceneGraph.
        message(FATAL_ERROR
          "Could not find [${dependency}] which is a dependency of [@LIB_NAME@]."
          " This may indicate a broken installation of OpenSceneGraph.")
    endif()

    include(${osg_component_dependency_target_file})
endforeach()

# Then we import our own target file
include(${CMAKE_CURRENT_LIST_DIR}/@LIB_NAME@-targets.cmake)
