TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 


CXXFILES = \
	AppVisitor.cpp\
	CameraManipulator.cpp\
	CullVisitor.cpp\
	NewCullVisitor.cpp\
	CullViewState.cpp\
	DisplayListVisitor.cpp\
	DisplayRequirementsVisitor.cpp\
	DepthSortedBin.cpp\
	DriveManipulator.cpp\
	FlightManipulator.cpp\
	IntersectVisitor.cpp\
	InsertImpostorsVisitor.cpp\
	NvTriStripObjects.cpp\
	Optimizer.cpp\
	RenderBin.cpp\
	RenderGraph.cpp\
	RenderLeaf.cpp\
	RenderStage.cpp\
	RenderStageLighting.cpp\
	RenderToTextureStage.cpp\
	SceneView.cpp\
	SceneViewManipulator.cpp\
	SmoothingVisitor.cpp\
	StateSetManipulator.cpp\
	Tesselator.cpp\
	TrackballManipulator.cpp\
	TransformCallback.cpp\
	TriStripVisitor.cpp\
	Version.cpp\

DEF     += -DOSGUTIL_LIBRARY

LIBS  += -losg $(GL_LIBS) $(OTHER_LIBS)

TARGET_BASENAME = osgUtil
LIB = $(LIB_PREFIX)$(TARGET_BASENAME).$(LIB_EXT)

include $(TOPDIR)/Make/makerules 
