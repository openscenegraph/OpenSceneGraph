TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 


CXXFILES = \
	AppVisitor.cpp\
	CullVisitor.cpp\
	DisplayListVisitor.cpp\
	DisplayRequirementsVisitor.cpp\
	InsertImpostorsVisitor.cpp\
	IntersectVisitor.cpp\
	NvTriStripObjects.cpp\
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
	Version.cpp\

DEF     += -DOSGUTIL_LIBRARY

LIBS  += -losg $(GL_LIBS) $(OTHER_LIBS)

TARGET_BASENAME = osgUtil
LIB = $(LIB_PREFIX)$(TARGET_BASENAME).$(LIB_EXT)

include $(TOPDIR)/Make/makerules 
