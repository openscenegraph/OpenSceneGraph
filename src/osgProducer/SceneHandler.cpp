//Open Producer - Copyright (C) 2002 Don Burns
//Distributed under the terms of the GNU LIBRARY GENERAL PUBLIC LICENSE (LGPL)
//as published by the Free Software Foundation.

#include <osgProducer/SceneHandler>
#include <Producer/Mutex>

using namespace osgUtil;
using namespace osgProducer;

SceneHandler::SceneHandler( osg::DisplaySettings *ds = NULL) :
    osgUtil::SceneView(ds)
{
    mm = new osg::RefMatrix;
    pm = new osg::RefMatrix;
}

void SceneHandler::init()
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


void SceneHandler::cull(Producer::Camera &cam) 
{
//     static Producer::Mutex mutex;
//     osg::notify(osg::INFO)<<"entering "<<this<<" cull."<<std::endl;
//     mutex.lock();
//     osg::notify(osg::INFO)<<"   running "<<this<<" cull."<<std::endl;

    pm->set(cam.getProjectionMatrix());
    mm->set(cam.getPositionAndAttitudeMatrix());
    setProjectionMatrix( pm.get() );
    setModelViewMatrix( mm.get() );

    int x, y;
    unsigned int w, h;
    cam.getProjectionRect( x, y, w, h );

    setViewport( x, y, w, h );

    SceneView::cull();

//     osg::notify(osg::INFO)<<"   done "<<this<<" cull."<<std::endl;
//     mutex.unlock();
//     osg::notify(osg::INFO)<<"   unlocked "<<this<<" cull."<<std::endl;
}

void SceneHandler::draw(Producer::Camera &) 
{
//     static Producer::Mutex mutex;
//     osg::notify(osg::INFO)<<"entering "<<this<<" draw."<<std::endl;
//     mutex.lock();
//     osg::notify(osg::INFO)<<"   running "<<this<<" draw."<<std::endl;
// 
    SceneView::draw();

//     osg::notify(osg::INFO)<<"   done "<<this<<" draw."<<std::endl;
//     mutex.unlock();
//     osg::notify(osg::INFO)<<"   unlocked "<<this<<" draw."<<std::endl;
}

void SceneHandler::setContextID( int id )
{
    getState()->setContextID( id );
}
