/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2003 Robert Osfield 
 *
 * This library is open source and may be redistributed and/or modified under  
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or 
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * OpenSceneGraph Public License for more details.
*/

#include <osgProducer/OsgSceneHandler>
#include <Producer/Mutex>

using namespace osgUtil;
using namespace osgProducer;

OsgSceneHandler::OsgSceneHandler( osg::DisplaySettings *ds) :
    osgUtil::SceneView(ds)
{
    mm = new osg::RefMatrix;
    pm = new osg::RefMatrix;
}

void OsgSceneHandler::init()
{
    static Producer::Mutex mutex;
    osg::notify(osg::INFO)<<"entering "<<this<<" init."<<std::endl;
    mutex.lock();
    osg::notify(osg::INFO)<<"   running "<<this<<" init."<<std::endl;

    SceneView::init();

    osg::notify(osg::INFO)<<"   done "<<this<<" init."<<std::endl;
    mutex.unlock();
    osg::notify(osg::INFO)<<"   unlocked "<<this<<" init."<<std::endl;
}

void OsgSceneHandler::clear(Producer::Camera& /*camera*/)
{
    // no-op right now as scene view manages its own cleaer.
}

void OsgSceneHandler::cull(Producer::Camera &cam) 
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

void OsgSceneHandler::draw(Producer::Camera &) 
{
    SceneView::draw();
}

void OsgSceneHandler::setContextID( int id )
{
    getState()->setContextID( id );
}
