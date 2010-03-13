
IF(DYNAMIC_OPENSCENEGRAPH)
    ADD_DEFINITIONS(-DOSGANIMATION_LIBRARY)
ELSE()
    ADD_DEFINITIONS(-DOSG_LIBRARY_STATIC)
ENDIF()

SET(LIB_NAME osgAnimation)


SET(HEADER_PATH ${OpenSceneGraph_SOURCE_DIR}/include/${LIB_NAME})
SET(LIB_PUBLIC_HEADERS
    ${HEADER_PATH}/Action
    ${HEADER_PATH}/ActionAnimation
    ${HEADER_PATH}/ActionBlendIn
    ${HEADER_PATH}/ActionBlendOut
    ${HEADER_PATH}/ActionCallback
    ${HEADER_PATH}/ActionStripAnimation
    ${HEADER_PATH}/ActionVisitor
    ${HEADER_PATH}/Animation
    ${HEADER_PATH}/AnimationManagerBase
    ${HEADER_PATH}/AnimationUpdateCallback
    ${HEADER_PATH}/BasicAnimationManager
    ${HEADER_PATH}/Bone
    ${HEADER_PATH}/BoneMapVisitor
    ${HEADER_PATH}/Channel
    ${HEADER_PATH}/CubicBezier
    ${HEADER_PATH}/EaseMotion
    ${HEADER_PATH}/Export
    ${HEADER_PATH}/FrameAction
    ${HEADER_PATH}/Interpolator
    ${HEADER_PATH}/Keyframe
    ${HEADER_PATH}/LinkVisitor
    ${HEADER_PATH}/MorphGeometry
    ${HEADER_PATH}/RigGeometry
    ${HEADER_PATH}/RigTransform
    ${HEADER_PATH}/RigTransformHardware
    ${HEADER_PATH}/RigTransformSoftware
    ${HEADER_PATH}/Sampler
    ${HEADER_PATH}/Skeleton
    ${HEADER_PATH}/StackedMatrixElement
    ${HEADER_PATH}/StackedQuaternionElement
    ${HEADER_PATH}/StackedRotateAxisElement
    ${HEADER_PATH}/StackedScaleElement
    ${HEADER_PATH}/StackedTransformElement
    ${HEADER_PATH}/StackedTranslateElement
    ${HEADER_PATH}/StackedTransform
    ${HEADER_PATH}/StatsVisitor
    ${HEADER_PATH}/StatsHandler
    ${HEADER_PATH}/Target
    ${HEADER_PATH}/Timeline
    ${HEADER_PATH}/TimelineAnimationManager
    ${HEADER_PATH}/UpdateBone
    ${HEADER_PATH}/UpdateMaterial
    ${HEADER_PATH}/UpdateMatrixTransform
    ${HEADER_PATH}/Vec3Packed
    ${HEADER_PATH}/VertexInfluence
)


ADD_LIBRARY(${LIB_NAME}
    ${OPENSCENEGRAPH_USER_DEFINED_DYNAMIC_OR_STATIC}
    ${LIB_PUBLIC_HEADERS}
    Action.cpp
    ActionAnimation.cpp
    ActionBlendIn.cpp
    ActionBlendOut.cpp
    ActionCallback.cpp
    ActionStripAnimation.cpp
    ActionVisitor.cpp
    Animation.cpp
    AnimationManagerBase.cpp
    BasicAnimationManager.cpp
    Bone.cpp
    BoneMapVisitor.cpp
    Channel.cpp
    LinkVisitor.cpp
    MorphGeometry.cpp
    RigGeometry.cpp
    RigTransformHardware.cpp
    RigTransformSoftware.cpp
    Skeleton.cpp
    StackedMatrixElement.cpp
    StackedQuaternionElement.cpp
    StackedRotateAxisElement.cpp
    StackedScaleElement.cpp
    StackedTransform.cpp
    StackedTranslateElement.cpp
    StatsVisitor.cpp
    StatsHandler.cpp
    Target.cpp
    TimelineAnimationManager.cpp
    Timeline.cpp
    UpdateBone.cpp
    UpdateMaterial.cpp
    UpdateMatrixTransform.cpp
    VertexInfluence.cpp
    ${OPENSCENEGRAPH_VERSIONINFO_RC}
)

LINK_INTERNAL(${LIB_NAME}
    osg
    osgText
    osgGA
    osgViewer
    OpenThreads
)

LINK_CORELIB_DEFAULT(${LIB_NAME})
INCLUDE(ModuleInstall OPTIONAL)
