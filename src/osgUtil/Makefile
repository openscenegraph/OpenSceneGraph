#!smake
include ../../Make/makedefs

C++FILES = \
        DisplayListVisitor.cpp\
	RenderVisitor.cpp\
	SceneView.cpp\
	IntersectVisitor.cpp\
        CameraManipulator.cpp\
        TrackballManipulator.cpp\
        FlightManipulator.cpp\
        DriveManipulator.cpp\
        Version.cpp\


TARGET_BASENAME = osgUtil


LIBS    = -ldl

LIB	= ../../lib/lib$(TARGET_BASENAME).so
#LIB	= ../../lib/lib$(TARGET_BASENAME).a

TARGET_LIB_FILES = lib$(TARGET_BASENAME).so
TARGET_INCLUDE_FILES = \
	osgUtil/CameraManipulator\
	osgUtil/DisplayListVisitor\
	osgUtil/DriveManipulator\
	osgUtil/Export\
	osgUtil/GUIActionAdapter\
	osgUtil/GUIEventAdapter\
	osgUtil/IntersectVisitor\
	osgUtil/RenderVisitor\
	osgUtil/SceneView\
	osgUtil/TrackballManipulator\
	osgUtil/FlightManipulator\
	osgUtil/Version\


C++FLAGS += -I ../../include

include ../../Make/makerules

