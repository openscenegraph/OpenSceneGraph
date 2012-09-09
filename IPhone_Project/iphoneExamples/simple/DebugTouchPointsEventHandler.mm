/*
 *  DebugTouchPointsEventHandler.cpp
 *  OSGIOS
 *
 *  Created by Stephan Huber on 13.09.10.
 *  Copyright 2010 Stephan Maximilian Huber, digital mind. All rights reserved.
 *
 */

#include "DebugTouchPointsEventHandler.h"
#include <iostream>

DebugTouchPointsEventHandler::DebugTouchPointsEventHandler(osg::Camera* hud_camera)
    : osgGA::GUIEventHandler()
    , _hudCamera(hud_camera)
{
}


bool DebugTouchPointsEventHandler::handle (const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa, osg::Object *, osg::NodeVisitor *)
{
	switch( ea.getEventType() ) {
        case osgGA::GUIEventAdapter::RESIZE:
            if (_hudCamera.valid()) {
                _hudCamera->setProjectionMatrix(osg::Matrix::ortho2D(0,ea.getWindowWidth(),0,ea.getWindowHeight()));
            }
            break;
            
		case osgGA::GUIEventAdapter::PUSH:
		case osgGA::GUIEventAdapter::DRAG:
		case osgGA::GUIEventAdapter::RELEASE:

			if (ea.isMultiTouchEvent()) 
			{
				osgGA::GUIEventAdapter::TouchData* data = ea.getTouchData();
				for(osgGA::GUIEventAdapter::TouchData::iterator i = data->begin(); i != data->end(); ++i) {
					osg::notify(osg::DEBUG_INFO) << ea.getEventType() << " id: " << i->id << " phase: " << i->phase << " " << i->x << "/" << i->y << " tapCount:" << i->tapCount << std::endl;
				}
				if (_node.valid()) updateDebugNode(data, ea.getWindowWidth(), ea.getWindowHeight());
			}
			return false;
			break;
	}
	return false;
}

void DebugTouchPointsEventHandler::createDebugNode()
{
	osg::Geode* geode = new osg::Geode();
	_geo = new osg::Geometry();
	_vertices = new osg::Vec3Array();
	_geo->setVertexArray(_vertices);
	
	_colors = new osg::Vec4Array();
	_geo->setColorArray(_colors);
	_geo->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
	
	
	_da = new osg::DrawArrays(GL_LINES);
	_geo->addPrimitiveSet(_da);
	
	
	
	geode->addDrawable(_geo);
	_geo->setUseDisplayList(false);
	_geo->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	
	_node = geode;
	updateDebugNode(NULL, 640, 480);
}


void DebugTouchPointsEventHandler::updateDebugNode(osgGA::GUIEventAdapter::TouchData* data, int width, int height)
{
	unsigned int num(data ? data->getNumTouchPoints() : 0);
	
	if (num == 0) {
		_vertices->resize(4);
		_colors->resize(4);
		for(unsigned int i = 0; i < 4; ++i) {
			(*_vertices)[i].set(0,0,0);
			(*_colors)[i].set(0,0,0, 0);
		
		}
		_da->setCount(_vertices->size());
		_geo->dirtyBound();
		return;
		
	}
	
	
	_vertices->resize(num * 4);
	_colors->resize(num * 4);
	
	unsigned int ndx(0);
	for(osgGA::GUIEventAdapter::TouchData::iterator i = data->begin(); i != data->end(); ++i, ndx+=4) 
	{
		(*_vertices)[ndx+0].set(i->x, 0, 0);
		(*_vertices)[ndx+1].set(i->x, height, 0);
		
        // the origin of touch coordinates is in the topleft corner of the screen
		(*_vertices)[ndx+2].set(0, height - i->y, 0);
		(*_vertices)[ndx+3].set(width, height - i->y, 0);
		
		for(unsigned k = ndx; k < ndx +4; ++k) {
			(*_colors)[k].set(
				(i->phase ==  osgGA::GUIEventAdapter::TOUCH_BEGAN) || (i->phase ==  osgGA::GUIEventAdapter::TOUCH_MOVED),
				(i->phase ==  osgGA::GUIEventAdapter::TOUCH_MOVED) || (i->phase ==  osgGA::GUIEventAdapter::TOUCH_STATIONERY),
				(i->phase ==  osgGA::GUIEventAdapter::TOUCH_ENDED),
				1.0);
		}
		
	}
	_da->setCount(_vertices->size());
	_geo->dirtyBound();
}