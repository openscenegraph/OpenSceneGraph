#!smake
include $(TOP)/Make/makedefs

C++FILES = \
	AppVisitor.cpp\
	CameraManipulator.cpp\
	CullVisitor.cpp\
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


TARGET_BASENAME = osgUtil


LIBS    = -L$(TOP)/lib -losg $(GL_LIBS)

LIB	= $(TOP)/lib/lib$(TARGET_BASENAME).$(SO_EXT)
#LIB	= $(TOP)/lib/lib$(TARGET_BASENAME).a

TARGET_LIB_FILES = lib$(TARGET_BASENAME).$(SO_EXT)
TARGET_INCLUDE_FILES = \
	osgUtil/AppVisitor\
	osgUtil/CameraManipulator\
	osgUtil/CullVisitor\
	osgUtil/CullViewState\
	osgUtil/DepthSortedBin\
	osgUtil/DisplayListVisitor\
	osgUtil/DisplayRequirementsVisitor\
	osgUtil/DriveManipulator\
	osgUtil/Export\
	osgUtil/FlightManipulator\
	osgUtil/GUIActionAdapter\
	osgUtil/GUIEventAdapter\
	osgUtil/GUIEventHandler\
	osgUtil/IntersectVisitor\
	osgUtil/InsertImpostorsVisitor\
	osgUtil/Optimizer\
	osgUtil/RenderBin\
	osgUtil/RenderGraph\
	osgUtil/RenderLeaf\
	osgUtil/RenderStage\
	osgUtil/RenderStageLighting\
	osgUtil/RenderToTextureStage\
	osgUtil/SceneView\
	osgUtil/SceneViewManipulator\
	osgUtil/SmoothingVisitor\
	osgUtil/StateSetManipulator\
	osgUtil/Tesselator\
	osgUtil/TrackballManipulator\
	osgUtil/TransformCallback\
	osgUtil/TriStripVisitor\
	osgUtil/Version\


C++FLAGS += -I $(TOP)/include -DOSGUTIL_LIBRARY

#LDFLAGS += --out-implib $(TOP)/lib/lib$(TARGET_BASENAME).a

include $(TOP)/Make/makerules

