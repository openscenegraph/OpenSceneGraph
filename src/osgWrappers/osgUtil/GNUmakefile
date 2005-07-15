TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	CubeMapGenerator.cpp\
	CullVisitor.cpp\
	DelaunayTriangulator.cpp\
	DisplayRequirementsVisitor.cpp\
	GLObjectsVisitor.cpp\
	HalfWayMapGenerator.cpp\
	HighlightMapGenerator.cpp\
	IntersectVisitor.cpp\
	Optimizer.cpp\
	ReflectionMapGenerator.cpp\
	RenderBin.cpp\
	RenderGraph.cpp\
	RenderLeaf.cpp\
	RenderStage.cpp\
	RenderStageLighting.cpp\
	RenderToTextureStage.cpp\
	SceneView.cpp\
	Simplifier.cpp\
	SmoothingVisitor.cpp\
	Statistics.cpp\
	TangentSpaceGenerator.cpp\
	Tesselator.cpp\
	TransformAttributeFunctor.cpp\
	TransformCallback.cpp\
	TriStripVisitor.cpp\
	UpdateVisitor.cpp\

LIBS   += -losgUtil -losg -losgIntrospection  $(GL_LIBS) $(OTHER_LIBS)

OPTF = -O

TARGET_BASENAME = osgUtil
include $(TOPDIR)/Make/cygwin_wrapper_def
WRAPPER = $(WRAPPER_PREFIX)$(TARGET_BASENAME).$(PLUGIN_EXT)

include $(TOPDIR)/Make/makerules 
