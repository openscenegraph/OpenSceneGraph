#!smake
include ../../Make/makedefs

C++FILES = \
	AppVisitor.cpp\
	CameraManipulator.cpp\
	CullVisitor.cpp\
	CullViewState.cpp\
	DisplayListVisitor.cpp\
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
        TriStripVisitor.cpp\
        Version.cpp\
	VisualsRequirementsVisitor.cpp\


TARGET_BASENAME = osgUtil


LIBS    = -L../../lib -losg -lGLU -lGL -lm

LIB	= ../../lib/lib$(TARGET_BASENAME).so
#LIB	= ../../lib/lib$(TARGET_BASENAME).a

TARGET_LIB_FILES = lib$(TARGET_BASENAME).so
TARGET_INCLUDE_FILES = \
	osgUtil/AppVisitor\
	osgUtil/CameraManipulator\
	osgUtil/CullVisitor\
	osgUtil/CullViewState\
	osgUtil/DepthSortedBin\
	osgUtil/DisplayListVisitor\
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
	osgUtil/TriStripVisitor\
	osgUtil/Version\
	osgUtil/VisualsRequirementsVisitor\


C++FLAGS += -I ../../include

include ../../Make/makerules

