TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 


CXXFILES = \
	UpdateVisitor.cpp\
	CullVisitor.cpp\
	DisplayListVisitor.cpp\
	DisplayRequirementsVisitor.cpp\
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
	SmoothingVisitor.cpp\
	Tesselator.cpp\
	TransformCallback.cpp\
	TriStripVisitor.cpp\
        TriStrip_tri_stripper.cpp\
	CubeMapGenerator.cpp\
	HalfWayMapGenerator.cpp\
	HighlightMapGenerator.cpp\
	Version.cpp\

DEF     += -DOSGUTIL_LIBRARY

LIBS  += -losg $(GL_LIBS) $(OTHER_LIBS)

TARGET_BASENAME = osgUtil
LIB = $(LIB_PREFIX)$(TARGET_BASENAME).$(LIB_EXT)

include $(TOPDIR)/Make/makerules 
