TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 


CXXFILES = \
	CubeMapGenerator.cpp\
	CullVisitor.cpp\
	DelaunayTriangulator.cpp\
	GLObjectsVisitor.cpp\
	DisplayRequirementsVisitor.cpp\
	HalfWayMapGenerator.cpp\
	HighlightMapGenerator.cpp\
	IntersectVisitor.cpp\
	Optimizer.cpp\
	RenderBin.cpp\
	StateGraph.cpp\
	RenderLeaf.cpp\
	RenderStage.cpp\
	PositionalStateContainer.cpp\
	SceneView.cpp\
	Simplifier.cpp\
	SmoothingVisitor.cpp\
	TangentSpaceGenerator.cpp\
	Tesselator.cpp\
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
