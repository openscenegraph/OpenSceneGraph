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
	IntersectionVisitor.cpp\
	LineSegmentIntersector.cpp\
	Optimizer.cpp\
	ReflectionMapGenerator.cpp\
	RenderBin.cpp\
	RenderStage.cpp\
	SceneView.cpp\
	Simplifier.cpp\
	SmoothingVisitor.cpp\
	Statistics.cpp\
	Tessellator.cpp\
	TransformAttributeFunctor.cpp\
	TriStripVisitor.cpp\
	UpdateVisitor.cpp\

LIBS   += -losgUtil -losg -losgIntrospection  $(GL_LIBS) $(OTHER_LIBS)

OPTF = -O

TARGET_BASENAME = osgUtil
include $(TOPDIR)/Make/cygwin_wrapper_def
WRAPPER = $(WRAPPER_PREFIX)$(TARGET_BASENAME).$(PLUGIN_EXT)

include $(TOPDIR)/Make/makerules 
