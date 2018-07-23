INCLUDE_DIRECTORIES( ${COLLADA_INCLUDE_DIR} ${COLLADA_INCLUDE_DIR}/1.4)


################################################################################
## Quieten warnings that a due to dae headers

IF(CMAKE_COMPILER_IS_GNUCXX)
    # Remove -Wextra flag as it barfs on ffmoeg headers
   STRING(REGEX REPLACE "-Wextra" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
ENDIF()

IF(CMAKE_COMPILER_IS_GNUCXX OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
   SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")
ENDIF()


SET(TARGET_SRC
    daeReader.cpp
    daeRAnimations.cpp
    daeRGeometry.cpp
    daeRMaterials.cpp
    daeRSceneObjects.cpp
    daeRSkinning.cpp
    daeRTransforms.cpp
    daeWAnimations.cpp
    daeWGeometry.cpp
    daeWMaterials.cpp
    daeWriter.cpp
    daeWSceneObjects.cpp
    daeWTransforms.cpp
    domSourceReader.cpp
    ReaderWriterDAE.cpp
)

SET(TARGET_H
    daeReader.h
    daeWriter.h
    domSourceReader.h
    ReaderWriterDAE.h
)


IF(COLLADA_DYNAMIC_LIBRARY AND COLLADA_STATIC_LIBRARY)
    OPTION(COLLADA_USE_STATIC "Set to ON to build OpenSceneGraph with static Collada support." OFF)
ELSEIF(COLLADA_STATIC_LIBRARY)
    SET(COLLADA_USE_STATIC "YES")
ELSE()
    SET(COLLADA_USE_STATIC "NO")
ENDIF()

ADD_DEFINITIONS(-DNO_BOOST)
ADD_DEFINITIONS(-DCOLLADA_DOM_SUPPORT141)

IF (COLLADA_DOM_2_4_OR_LATER)
    ADD_DEFINITIONS(-DCOLLADA_DOM_2_4_OR_LATER)
ENDIF()

IF(COLLADA_USE_STATIC)
    IF(WIN32)
        ADD_DEFINITIONS(-DBOOST_ALL_NO_LIB)
        SET(TARGET_LIBRARIES_VARS
            COLLADA_STATIC_LIBRARY
            COLLADA_LIBXML_LIBRARY
            COLLADA_ZLIB_LIBRARY
            COLLADA_PCRECPP_LIBRARY
            COLLADA_PCRE_LIBRARY
            COLLADA_MINIZIP_LIBRARY
            COLLADA_BOOST_FILESYSTEM_LIBRARY
            COLLADA_BOOST_SYSTEM_LIBRARY)
    ELSE()
        SET(TARGET_LIBRARIES_VARS
            COLLADA_STATIC_LIBRARY
            COLLADA_LIBXML_LIBRARY
            COLLADA_PCRECPP_LIBRARY
            COLLADA_PCRE_LIBRARY
            COLLADA_BOOST_FILESYSTEM_LIBRARY
            COLLADA_BOOST_SYSTEM_LIBRARY)
    ENDIF()
ELSE()
    IF(WIN32)
        ADD_DEFINITIONS(-DDOM_DYNAMIC)
        SET(TARGET_LIBRARIES_VARS COLLADA_DYNAMIC_LIBRARY)
    ELSE()
        SET(TARGET_LIBRARIES_VARS
                    COLLADA_DYNAMIC_LIBRARY
                    COLLADA_BOOST_FILESYSTEM_LIBRARY)
    ENDIF()

ENDIF()

SET(TARGET_ADDED_LIBRARIES osgSim osgAnimation)

IF(CMAKE_COMPILER_IS_GNUCXX)
    # Remove -Wshadow flag as it barfs on ffmoeg headers
   STRING(REGEX REPLACE "-Wshadow" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
ENDIF()

#### end var setup  ###
SETUP_PLUGIN(dae dae)
