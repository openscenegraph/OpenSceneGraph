//Open Producer - Copyright (C) 2002 Don Burns
//Distributed under the terms of the GNU LIBRARY GENERAL PUBLIC LICENSE (LGPL)
//as published by the Free Software Foundation.

#include <osgProducer/SceneHandler>

using namespace osgUtil;
using namespace osgProducer;

SceneHandler::SceneHandler( osg::DisplaySettings *ds = NULL) :
    osgUtil::SceneView(ds)
{
    mm = new osg::RefMatrix;
    pm = new osg::RefMatrix;
}

void SceneHandler::cull(Producer::Camera &cam) 
{
    pm->set(cam.getProjectionMatrix());
    mm->set(cam.getPositionAndAttitudeMatrix());
    setProjectionMatrix( pm.get() );
    setModelViewMatrix( mm.get() );

    int x, y;
    unsigned int w, h;
    cam.getProjectionRect( x, y, w, h );

    setViewport( x, y, w, h );

    SceneView::cull();
}

void SceneHandler::draw(Producer::Camera &) 
{
    SceneView::draw();
}

void SceneHandler::setContextID( int id )
{
    getState()->setContextID( id );
}
