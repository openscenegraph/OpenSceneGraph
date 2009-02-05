
IF   (DYNAMIC_OPENSCENEGRAPH)
    ADD_DEFINITIONS(-DOSGANIMATION_LIBRARY)
ELSE (DYNAMIC_OPENSCENEGRAPH)
    ADD_DEFINITIONS(-DOSG_LIBRARY_STATIC)
ENDIF(DYNAMIC_OPENSCENEGRAPH)

SET(LIB_NAME osgAnimation)


SET(HEADER_PATH ${OpenSceneGraph_SOURCE_DIR}/include/${LIB_NAME})
SET(LIB_PUBLIC_HEADERS
    ${HEADER_PATH}/Export
    ${HEADER_PATH}/Bone
    ${HEADER_PATH}/Skeleton
    ${HEADER_PATH}/Channel
    ${HEADER_PATH}/Sampler
    ${HEADER_PATH}/Interpolator
    ${HEADER_PATH}/Target
    ${HEADER_PATH}/Animation
    ${HEADER_PATH}/Keyframe
    ${HEADER_PATH}/Skinning
    ${HEADER_PATH}/CubicBezier
    ${HEADER_PATH}/Vec3Packed
    ${HEADER_PATH}/BasicAnimationManager
    ${HEADER_PATH}/TimelineAnimationManager
    ${HEADER_PATH}/AnimationManagerBase
    ${HEADER_PATH}/UpdateCallback
    ${HEADER_PATH}/LinkVisitor
    ${HEADER_PATH}/VertexInfluence
    ${HEADER_PATH}/EaseMotion
    ${HEADER_PATH}/Assert
    ${HEADER_PATH}/Timeline
    ${HEADER_PATH}/RigGeometry
)


ADD_LIBRARY(${LIB_NAME}
    ${OPENSCENEGRAPH_USER_DEFINED_DYNAMIC_OR_STATIC}
    ${LIB_PUBLIC_HEADERS}
    Channel.cpp
    Target.cpp
    Animation.cpp
    Bone.cpp
    RigGeometry.cpp
    AnimationManager.cpp
    BasicAnimationManager.cpp
    TimelineAnimationManager.cpp
    AnimationManagerBase.cpp
    Skeleton.cpp
    VertexInfluence.cpp
    UpdateCallback.cpp
    Timeline.cpp
)

LINK_INTERNAL(${LIB_NAME}
    osg
    OpenThreads
)

LINK_CORELIB_DEFAULT(${LIB_NAME})
INCLUDE(ModuleInstall OPTIONAL)
