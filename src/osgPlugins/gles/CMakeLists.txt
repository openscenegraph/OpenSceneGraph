SET(TARGET_SRC
    ReaderWriterGLES.cpp
    OpenGLESGeometryOptimizer.cpp
    TriangleStripVisitor.cpp
    IndexMeshVisitor.cpp
    UnIndexMeshVisitor.cpp)

SET(TARGET_H
    AABBonBoneVisitor
    AnimationCleanerVisitor
    BindPerVertexVisitor
    debug
    DetachPrimitiveVisitor
    DisableAnimationVisitor
    DrawArrayVisitor
    EdgeIndexFunctor
    GeometryArray
    GeometryInspector
    GeometrySplitterVisitor
    GeometryUniqueVisitor
    glesUtil
    IndexMeshVisitor
    LimitMorphTargetCount
    Line
    LineIndexFunctor
    MostInfluencedGeometryByBone
    OpenGLESGeometryOptimizer
    PointIndexFunctor
    PreTransformVisitor
    PrimitiveIndexors
    RigAnimationVisitor
    RigAttributesVisitor
    SmoothNormalVisitor
    StatLogger
    SubGeometry
    TangentSpaceVisitor
    TriangleMeshGraph
    TriangleStripVisitor
    UnIndexMeshVisitor
    WireframeVisitor
)

#### end var setup  ###
SET(TARGET_ADDED_LIBRARIES
    osgUtil osgAnimation)
SETUP_PLUGIN(gles)
