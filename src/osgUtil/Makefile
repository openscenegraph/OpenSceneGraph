TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 


CXXFILES = \
	AppVisitor.cpp\
	CameraManipulator.cpp\
	CullViewState.cpp\
	CullVisitor.cpp\
	DepthSortedBin.cpp\
	DisplayListVisitor.cpp\
	DisplayRequirementsVisitor.cpp\
	DriveManipulator.cpp\
	FlightManipulator.cpp\
	InsertImpostorsVisitor.cpp\
	IntersectVisitor.cpp\
	NewCullVisitor.cpp\
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
