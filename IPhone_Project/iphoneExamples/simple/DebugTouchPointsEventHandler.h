/*
 *  DebugTouchPointsEventHandler.h
 *  OSGIOS
 *
 *  Created by Stephan Huber on 13.09.10.
 *  Copyright 2010 Stephan Maximilian Huber, digital mind. All rights reserved.
 *
 */

#ifndef DEBUG_TOUCH_POINT_EVENT_HANDLER_HEADER
#define DEBUG_TOUCH_POINT_EVENT_HANDLER_HEADER

#include <osgGA/GUIEventHandler>
#include <osg/Geode>
#include <osg/Geometry>



class DebugTouchPointsEventHandler : public osgGA::GUIEventHandler {
public:
    DebugTouchPointsEventHandler(osg::Camera* hud_camera);
	virtual bool handle (const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa, osg::Object *, osg::NodeVisitor *);
	
	osg::Node* getDebugNode() { if (!_node.valid()) createDebugNode(); return _node.get(); }

private:
	void createDebugNode();
	void updateDebugNode(osgGA::GUIEventAdapter::TouchData* data, int w, int h);
	
	osg::ref_ptr<osg::Node> _node;
	osg::ref_ptr<osg::Vec3Array>			_vertices;
	osg::ref_ptr<osg::Vec4Array>			_colors;
	osg::ref_ptr<osg::DrawArrays>           _da;
	osg::ref_ptr<osg::Geometry>             _geo;
    osg::observer_ptr<osg::Camera>          _hudCamera;
};


#endif