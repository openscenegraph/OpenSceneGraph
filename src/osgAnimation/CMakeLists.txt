
IF   (DYNAMIC_OPENSCENEGRAPH)
    ADD_DEFINITIONS(-DOSGANIMATION_LIBRARY)
ELSE (DYNAMIC_OPENSCENEGRAPH)
    ADD_DEFINITIONS(-DOSG_LIBRARY_STATIC)
ENDIF(DYNAMIC_OPENSCENEGRAPH)

SET(LIB_NAME osgAnimation)


SET(HEADER_PATH ${OpenSceneGraph_SOURCE_DIR}/include/${LIB_NAME})
SET(LIB_PUBLIC_HEADERS
    ${HEADER_PATH}/Animation
    ${HEADER_PATH}/AnimationManagerBase
    ${HEADER_PATH}/Assert
    ${HEADER_PATH}/BasicAnimationManager
    ${HEADER_PATH}/Bone
    ${HEADER_PATH}/Channel
    ${HEADER_PATH}/CubicBezier
    ${HEADER_PATH}/EaseMotion
    ${HEADER_PATH}/Export
    ${HEADER_PATH}/Interpolator
    ${HEADER_PATH}/Keyframe
    ${HEADER_PATH}/LinkVisitor
    ${HEADER_PATH}/MorphGeometry
    ${HEADER_PATH}/RigGeometry
    ${HEADER_PATH}/Sampler
    ${HEADER_PATH}/Skeleton
    ${HEADER_PATH}/Skinning
    ${HEADER_PATH}/Target
    ${HEADER_PATH}/Timeline
    ${HEADER_PATH}/TimelineAnimationManager
    ${HEADER_PATH}/UpdateCallback
    ${HEADER_PATH}/Vec3Packed
    ${HEADER_PATH}/VertexInfluence
)


ADD_LIBRARY(${LIB_NAME}
    ${OPENSCENEGRAPH_USER_DEFINED_DYNAMIC_OR_STATIC}
    ${LIB_PUBLIC_HEADERS}
    Animation.cpp
    AnimationManagerBase.cpp
    AnimationManager.cpp
    BasicAnimationManager.cpp
    Bone.cpp
    Channel.cpp
    MorphGeometry.cpp
    RigGeometry.cpp
    Skeleton.cpp
    Target.cpp
    TimelineAnimationManager.cpp
    Timeline.cpp
    UpdateCallback.cpp
    VertexInfluence.cpp
    ${OPENSCENEGRAPH_VERSIONINFO_RC}
)

LINK_INTERNAL(${LIB_NAME}
    osg
    OpenThreads
)

LINK_CORELIB_DEFAULT(${LIB_NAME})
INCLUDE(ModuleInstall OPTIONAL)
