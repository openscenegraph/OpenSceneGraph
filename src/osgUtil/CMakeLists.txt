
# FIXME: For OS X, need flag for Framework or dylib
IF(DYNAMIC_OPENSCENEGRAPH)
    ADD_DEFINITIONS(-DOSGUTIL_LIBRARY)
ELSE()
    ADD_DEFINITIONS(-DOSG_LIBRARY_STATIC)
ENDIF()

SET(LIB_NAME osgUtil)
SET(HEADER_PATH ${OpenSceneGraph_SOURCE_DIR}/include/${LIB_NAME})
SET(TARGET_H
    ${HEADER_PATH}/ConvertVec
    ${HEADER_PATH}/CubeMapGenerator
    ${HEADER_PATH}/CullVisitor
    ${HEADER_PATH}/DelaunayTriangulator
    ${HEADER_PATH}/DisplayRequirementsVisitor
    ${HEADER_PATH}/DrawElementTypeSimplifier
    ${HEADER_PATH}/EdgeCollector
    ${HEADER_PATH}/Export
    ${HEADER_PATH}/GLObjectsVisitor
    ${HEADER_PATH}/HalfWayMapGenerator
    ${HEADER_PATH}/HighlightMapGenerator
    ${HEADER_PATH}/IntersectionVisitor
    ${HEADER_PATH}/IntersectVisitor
    ${HEADER_PATH}/IncrementalCompileOperation
    ${HEADER_PATH}/LineSegmentIntersector
    ${HEADER_PATH}/MeshOptimizers
    ${HEADER_PATH}/OperationArrayFunctor
    ${HEADER_PATH}/Optimizer
    ${HEADER_PATH}/PerlinNoise
    ${HEADER_PATH}/PlaneIntersector
    ${HEADER_PATH}/PolytopeIntersector
    ${HEADER_PATH}/PositionalStateContainer
    ${HEADER_PATH}/PrintVisitor
    ${HEADER_PATH}/RayIntersector
    ${HEADER_PATH}/ReflectionMapGenerator
    ${HEADER_PATH}/RenderBin
    ${HEADER_PATH}/RenderLeaf
    ${HEADER_PATH}/RenderStage
    ${HEADER_PATH}/ReversePrimitiveFunctor
    ${HEADER_PATH}/SceneView
    ${HEADER_PATH}/SceneGraphBuilder
    ${HEADER_PATH}/ShaderGen
    ${HEADER_PATH}/Simplifier
    ${HEADER_PATH}/SmoothingVisitor
    ${HEADER_PATH}/StateGraph
    ${HEADER_PATH}/Statistics
    ${HEADER_PATH}/TangentSpaceGenerator
    ${HEADER_PATH}/Tessellator
    ${HEADER_PATH}/TransformAttributeFunctor
    ${HEADER_PATH}/TransformCallback
    ${HEADER_PATH}/TriStripVisitor
    ${HEADER_PATH}/UpdateVisitor
    ${HEADER_PATH}/Version
)

SET(TARGET_SRC
    CubeMapGenerator.cpp
    CullVisitor.cpp
    DelaunayTriangulator.cpp
    DisplayRequirementsVisitor.cpp
    DrawElementTypeSimplifier.cpp
    EdgeCollector.cpp
    GLObjectsVisitor.cpp
    HalfWayMapGenerator.cpp
    HighlightMapGenerator.cpp
    IntersectionVisitor.cpp
    IntersectVisitor.cpp
    IncrementalCompileOperation.cpp
    LineSegmentIntersector.cpp
    MeshOptimizers.cpp
    Optimizer.cpp
    PerlinNoise.cpp
    PlaneIntersector.cpp
    PolytopeIntersector.cpp
    PositionalStateContainer.cpp
    PrintVisitor.cpp
    RayIntersector.cpp
    RenderBin.cpp
    RenderLeaf.cpp
    RenderStage.cpp
    ReversePrimitiveFunctor.cpp
    SceneView.cpp
    ShaderGen.cpp
    Simplifier.cpp
    SmoothingVisitor.cpp
    SceneGraphBuilder.cpp
    StateGraph.cpp
    Statistics.cpp
    TangentSpaceGenerator.cpp
    Tessellator.cpp
    TransformAttributeFunctor.cpp
    TransformCallback.cpp

    tristripper/include/detail/graph_array.h
    tristripper/include/detail/types.h
    tristripper/include/detail/cache_simulator.h
    tristripper/include/detail/policy.h
    tristripper/include/detail/heap_array.h
    tristripper/include/detail/connectivity_graph.h
    tristripper/include/tri_stripper.h
    tristripper/include/public_types.h
    tristripper/src/tri_stripper.cpp
    tristripper/src/policy.cpp
    tristripper/src/connectivity_graph.cpp

    TriStripVisitor.cpp
    UpdateVisitor.cpp
    Version.cpp
    ${OPENSCENEGRAPH_VERSIONINFO_RC}
)

INCLUDE_DIRECTORIES("tristripper/include")

SET(TARGET_LIBRARIES
    osg
    OpenThreads
)

SETUP_LIBRARY(${LIB_NAME})

