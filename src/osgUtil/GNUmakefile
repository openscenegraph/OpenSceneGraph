TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 


CXXFILES = \
	CubeMapGenerator.cpp\
	CullVisitor.cpp\
	DelaunayTriangulator.cpp\
	DisplayRequirementsVisitor.cpp\
	GLObjectsVisitor.cpp\
	HalfWayMapGenerator.cpp\
	HighlightMapGenerator.cpp\
	IntersectionVisitor.cpp\
	IntersectVisitor.cpp\
	LineSegmentIntersector.cpp\
	Optimizer.cpp\
	PlaneIntersector.cpp\
	PolytopeIntersector.cpp\
	PositionalStateContainer.cpp\
	RenderBin.cpp\
	RenderLeaf.cpp\
	RenderStage.cpp\
	SceneView.cpp\
	Simplifier.cpp\
	SmoothingVisitor.cpp\
	StateGraph.cpp\
	Statistics.cpp\
	TangentSpaceGenerator.cpp\
	Tessellator.cpp\
	TransformAttributeFunctor.cpp\
	TransformCallback.cpp\
        TriStrip_tri_stripper.cpp\
	TriStripVisitor.cpp\
	UpdateVisitor.cpp\
	Version.cpp\

DEF     += -DOSGUTIL_LIBRARY

LIBS  += -losg $(GL_LIBS) $(OTHER_LIBS)

TARGET_BASENAME = osgUtil
LIB = $(LIB_PREFIX)$(TARGET_BASENAME).$(LIB_EXT)

include $(TOPDIR)/Make/makerules 
