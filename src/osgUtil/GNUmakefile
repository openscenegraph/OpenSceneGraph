TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 


CXXFILES = \
	CubeMapGenerator.cpp\
	CullVisitor.cpp\
	DelaunayTriangulator.cpp\
	DisplayListVisitor.cpp\
	DisplayRequirementsVisitor.cpp\
	HalfWayMapGenerator.cpp\
	HighlightMapGenerator.cpp\
	InsertImpostorsVisitor.cpp\
	IntersectVisitor.cpp\
	Optimizer.cpp\
	RenderBin.cpp\
	RenderGraph.cpp\
	RenderLeaf.cpp\
	RenderStage.cpp\
	RenderStageLighting.cpp\
	RenderToTextureStage.cpp\
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
