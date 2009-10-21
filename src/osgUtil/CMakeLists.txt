
# FIXME: For OS X, need flag for Framework or dylib
IF   (DYNAMIC_OPENSCENEGRAPH)
    ADD_DEFINITIONS(-DOSGUTIL_LIBRARY)
ELSE (DYNAMIC_OPENSCENEGRAPH)
    ADD_DEFINITIONS(-DOSG_LIBRARY_STATIC)
ENDIF(DYNAMIC_OPENSCENEGRAPH)


#
# MSFBO resolve blit to depth buffer control.
#
OPTION(OSG_MULTISAMPLE_FBO_RESOLVE_IGNORES_DEPTH "Set to ON to ignore depth when resolving multisampled FBO rendering. This is a workaround for a crash on OS X with NVIDIA driver 1.5.49." OFF)
IF(OSG_MULTISAMPLE_FBO_RESOLVE_IGNORES_DEPTH)
    ADD_DEFINITIONS(-DOSG_MULTISAMPLE_FBO_RESOLVE_IGNORES_DEPTH)
ENDIF(OSG_MULTISAMPLE_FBO_RESOLVE_IGNORES_DEPTH)


SET(LIB_NAME osgUtil)
SET(HEADER_PATH ${OpenSceneGraph_SOURCE_DIR}/include/${LIB_NAME})
SET(LIB_PUBLIC_HEADERS
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
    ${HEADER_PATH}/LineSegmentIntersector
    ${HEADER_PATH}/OperationArrayFunctor
    ${HEADER_PATH}/Optimizer
    ${HEADER_PATH}/PlaneIntersector
    ${HEADER_PATH}/PolytopeIntersector
    ${HEADER_PATH}/PositionalStateContainer
    ${HEADER_PATH}/ReflectionMapGenerator
    ${HEADER_PATH}/RenderBin
    ${HEADER_PATH}/RenderLeaf
    ${HEADER_PATH}/RenderStage
    ${HEADER_PATH}/ReversePrimitiveFunctor
    ${HEADER_PATH}/SceneView
    ${HEADER_PATH}/SceneGraphBuilder
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

ADD_LIBRARY(${LIB_NAME}
    ${OPENSCENEGRAPH_USER_DEFINED_DYNAMIC_OR_STATIC}
    ${LIB_PUBLIC_HEADERS}
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
    LineSegmentIntersector.cpp
    Optimizer.cpp
    PlaneIntersector.cpp
    PolytopeIntersector.cpp
    PositionalStateContainer.cpp
    RenderBin.cpp
    RenderLeaf.cpp
    RenderStage.cpp
    ReversePrimitiveFunctor.cpp
    SceneView.cpp
    Simplifier.cpp
    SmoothingVisitor.cpp
    SceneGraphBuilder.cpp
    StateGraph.cpp
    Statistics.cpp
    TangentSpaceGenerator.cpp
    Tessellator.cpp
    TransformAttributeFunctor.cpp
    TransformCallback.cpp
    TriStrip_graph_array.h
    TriStrip_heap_array.h
    TriStrip_tri_stripper.cpp
    TriStrip_tri_stripper.h
    TriStripVisitor.cpp
    UpdateVisitor.cpp
    Version.cpp
)

LINK_INTERNAL(${LIB_NAME}     
    osg
    OpenThreads
)
LINK_CORELIB_DEFAULT(${LIB_NAME})

INCLUDE(ModuleInstall OPTIONAL)
