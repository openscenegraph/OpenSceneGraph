INCLUDE_DIRECTORIES(${FBX_ROOT}/include)

SET(TARGET_SRC
    fbxRAnimation.cpp
    fbxRCamera.cpp
    fbxRLight.cpp
    fbxRMesh.cpp
    fbxRNode.cpp
    ReaderWriterFBX.cpp
    WriterNodeVisitor.cpp
    fbxMaterialToOsgStateSet.cpp
)

SET(TARGET_H 
    fbxRAnimation.h
    fbxRCamera.h
    fbxRLight.h
    fbxRMesh.h
    fbxRNode.h
    ReaderWriterFBX.h
    WriterNodeVisitor.h
    fbxMaterialToOsgStateSet.h
)

ADD_DEFINITIONS(-DKFBX_PLUGIN -DKFBX_FBXSDK -DKFBX_NODLL)

IF(WIN32)
    SET(TARGET_EXTERNAL_LIBRARIES wininet)
ENDIF(WIN32)

IF(APPLE)
    FIND_LIBRARY(SYS_CONFIG_LIBRARY SystemConfiguration )
    MARK_AS_ADVANCED(SYS_CONFIG_LIBRARY)
    SET(TARGET_EXTERNAL_LIBRARIES iconv ${SYS_CONFIG_LIBRARY})
ENDIF(APPLE)

SET(TARGET_LIBRARIES_VARS FBX_LIBRARY)
SET(TARGET_ADDED_LIBRARIES osgAnimation) 

#### end var setup  ###
SETUP_PLUGIN(fbx)
